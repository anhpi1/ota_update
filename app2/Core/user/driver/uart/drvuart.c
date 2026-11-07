#include "drvuart.h"


static drvuart_Handle *sp_uart_instances[DRVUART_MAX_INSTANCES] = {NULL};

static void drvuart_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);
static void drvuart_TxCpltCallback(UART_HandleTypeDef *huart);
static void drvuart_ErrorCallback(UART_HandleTypeDef *huart);
static drvuart_Handle* drvuart_FindInstance(UART_HandleTypeDef *huart);

drvuart_Status drvuart_Init(drvuart_Handle *handle, UART_HandleTypeDef *huart)
{

    drvuart_Status status = drvuart_OK;
    if (handle == NULL || huart == NULL) return drvuart_ERROR;

    if(HAL_UART_GetState(huart) != HAL_UART_STATE_READY ) return drvuart_ERROR;

    // Find an available instance
    int instanceIndex = -1;
    for (int i = 0; i < DRVUART_MAX_INSTANCES; i++) {
        if (sp_uart_instances[i] == NULL) {
            instanceIndex = i;
            break;
        }
    }
    if (instanceIndex == -1) return drvuart_ERROR; // No available instance

    sp_uart_instances[instanceIndex] = handle;
    handle->huart = huart;

    handle->tx.mutex_lock_task = osMutexNew(NULL);

    handle->tx.sem_data_done = osSemaphoreNew(1, 0, NULL);

    handle->rx.mutex_lock_task = osMutexNew(NULL);

    handle->rx.sem_data_done = osSemaphoreNew(1, 0, NULL);
  
    if (HAL_UART_RegisterCallback(handle->huart, HAL_UART_TX_COMPLETE_CB_ID, drvuart_TxCpltCallback) != HAL_OK) status = drvuart_ERROR;
    if (HAL_UART_RegisterRxEventCallback(handle->huart, drvuart_RxEventCallback) != HAL_OK) status = drvuart_ERROR;
    if (HAL_UART_RegisterCallback(handle->huart, HAL_UART_ERROR_CB_ID, drvuart_ErrorCallback) != HAL_OK) status = drvuart_ERROR;

    status = drvuart_CheckHealth(handle);
    if(status != drvuart_OK) drvuart_DeInit(handle); // Clean up if initialization failed

    return status;
}

drvuart_Status drvuart_DeInit(drvuart_Handle *handle)
{
    drvuart_Status status = drvuart_OK;
    osStatus_t stat = osOK;



    if (handle == NULL) return drvuart_ERROR;

    if (handle->tx.mutex_lock_task != NULL) {
        stat = osMutexDelete(handle->tx.mutex_lock_task);
        handle->tx.mutex_lock_task = NULL;
    }

    if (handle->tx.sem_data_done != NULL) {
        stat = osSemaphoreDelete(handle->tx.sem_data_done);
        handle->tx.sem_data_done = NULL;
    }

    if (handle->rx.mutex_lock_task != NULL) {
        stat = osMutexDelete(handle->rx.mutex_lock_task);
        handle->rx.mutex_lock_task = NULL;
    }

    if (handle->rx.sem_data_done != NULL) {
        stat = osSemaphoreDelete(handle->rx.sem_data_done);
        handle->rx.sem_data_done = NULL;
    }

    if(handle->huart != NULL) {
        HAL_UART_UnRegisterCallback(handle->huart, HAL_UART_TX_COMPLETE_CB_ID);
        HAL_UART_UnRegisterRxEventCallback(handle->huart);
        HAL_UART_UnRegisterCallback(handle->huart, HAL_UART_ERROR_CB_ID);
        handle->huart = NULL;
    }

    for (int i = 0; i < DRVUART_MAX_INSTANCES; i++) {
        if (sp_uart_instances[i] == handle) {
            sp_uart_instances[i] = NULL;
            break;
        }
    }

    if (stat != osOK) status = drvuart_ERROR;


    return status;
}

drvuart_Status drvuart_CheckHealth(drvuart_Handle *handle)
{
    if (handle == NULL) return drvuart_ERROR;
    if (handle->huart                   == NULL                     || 
        handle->tx.mutex_lock_task      == NULL                     || 
        handle->tx.sem_data_done        == NULL                     ||
        handle->rx.mutex_lock_task      == NULL                     || 
        handle->rx.sem_data_done        == NULL                     ||
        handle->huart->TxCpltCallback   != drvuart_TxCpltCallback   ||
        handle->huart->RxEventCallback  != drvuart_RxEventCallback  ||   
        handle->huart->ErrorCallback    != drvuart_ErrorCallback        ) {
        return drvuart_ERROR;
    }
    HAL_UART_StateTypeDef state = HAL_UART_GetState(handle->huart);
    if (state == HAL_UART_STATE_RESET || state == HAL_UART_STATE_ERROR) {
        return drvuart_ERROR;
    }
    return drvuart_OK;
}

drvuart_Status drvuart_Transmit(drvuart_Handle *handle, const uint8_t *data, uint16_t length, uint32_t timeout)
{
    drvuart_Status status = drvuart_OK;
    if (drvuart_CheckHealth(handle) != drvuart_OK) return drvuart_ERROR;
    if (data == NULL || length == 0) return drvuart_ERROR;
    if (length > DRVUART_MAX_DATA_LENGTH) return drvuart_ERROR;

    if(osMutexAcquire(handle->tx.mutex_lock_task, timeout) != osOK) return drvuart_BUSY;

    // Đảm bảo rằng không còn dữ liệu nào đang chờ xử lý
    while (osSemaphoreAcquire(handle->tx.sem_data_done, 0) == osOK);
    
    memcpy(handle->tx.data, data, length);
    handle->tx.length = length;
    
    if(HAL_UART_Transmit_DMA(handle->huart, handle->tx.data, handle->tx.length) != HAL_OK) status = drvuart_ERROR;
    else if(osSemaphoreAcquire(handle->tx.sem_data_done, timeout) != osOK) status = drvuart_ERROR;

    

    if (status != drvuart_OK) {
        // Nếu có lỗi xảy ra, hủy việc truyền dữ liệu
        HAL_UART_AbortTransmit(handle->huart);
    }
    osMutexRelease(handle->tx.mutex_lock_task);
    return status;
}
drvuart_Status drvuart_Receive(drvuart_Handle *handle, uint8_t *data, uint16_t *length, uint32_t timeout)
{
    
    drvuart_Status status = drvuart_OK;

    if (drvuart_CheckHealth(handle) != drvuart_OK) return drvuart_ERROR;
    if (data == NULL || length == NULL) return drvuart_ERROR;

    if(osMutexAcquire(handle->rx.mutex_lock_task, timeout) != osOK) return drvuart_BUSY;

    // Đảm bảo rằng không còn dữ liệu nào đang chờ xử lý
    while (osSemaphoreAcquire(handle->rx.sem_data_done, 0) == osOK);
    
    // Bắt đầu nhận dữ liệu bằng DMA
    if(HAL_UARTEx_ReceiveToIdle_DMA(handle->huart, handle->rx.data, DRVUART_MAX_DATA_LENGTH) != HAL_OK) status = drvuart_ERROR;
    else if(osSemaphoreAcquire(handle->rx.sem_data_done, timeout) != osOK) status = drvuart_ERROR;
        else
        {
             memcpy(data, handle->rx.data, handle->rx.length);
            *length = handle->rx.length;
        }
    

    if (status != drvuart_OK) {
        // Nếu có lỗi xảy ra, hủy việc nhận dữ liệu
        HAL_UART_AbortReceive(handle->huart);
    }
    osMutexRelease(handle->rx.mutex_lock_task);
    return status;
}

static void drvuart_TxCpltCallback(UART_HandleTypeDef *huart)
{
    drvuart_Handle *handle = drvuart_FindInstance(huart);
    if (handle == NULL) return;
    if (huart == handle->huart) {
        // Nhả Semaphore từ ngắt, đánh thức Task dậy
        osSemaphoreRelease(handle->tx.sem_data_done);
        
    }
}

static void drvuart_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (HAL_UARTEx_GetRxEventType(huart) == HAL_UART_RXEVENT_HT) return;
    drvuart_Handle *handle = drvuart_FindInstance(huart);
    if (handle == NULL) return;
    if (huart == handle->huart) {
        // Nhả Semaphore từ ngắt, đánh thức Task dậy
        handle->rx.length = Size;
        osSemaphoreRelease(handle->rx.sem_data_done);
    }
}

static void drvuart_ErrorCallback(UART_HandleTypeDef *huart)
{
    drvuart_Handle *handle = drvuart_FindInstance(huart);
    if (handle == NULL) return;
    if (huart == handle->huart) {
        osSemaphoreRelease(handle->tx.sem_data_done);
        osSemaphoreRelease(handle->rx.sem_data_done);
    }
}

static drvuart_Handle* drvuart_FindInstance(UART_HandleTypeDef *huart)
{
    if (huart == NULL) return NULL;
    for (int i = 0; i < DRVUART_MAX_INSTANCES; i++) {
        if (sp_uart_instances[i] != NULL && sp_uart_instances[i]->huart == huart) {
            return sp_uart_instances[i];
        }
    }
    return NULL;
}