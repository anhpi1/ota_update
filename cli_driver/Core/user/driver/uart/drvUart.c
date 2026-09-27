#include "drvUart.h"
#include "cmsis_os.h"


static UART_HandleTypeDef *drvuart_handle;
static drvuart_Data drvuart_Tx = {0};
static drvuart_Data drvuart_Rx = {0};


static void drvuart_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);
static void drvuart_TxCpltCallback(UART_HandleTypeDef *huart);
static void drvuart_ErrorCallback(UART_HandleTypeDef *huart);

drvuart_Status drvuart_Init(UART_HandleTypeDef *huart)
{
    drvuart_Status status = DRVUART_OK;
    if (huart == NULL) return DRVUART_ERROR;
    drvuart_handle = huart;

    drvuart_Tx.mutex_lock_task = osMutexNew(NULL);

    drvuart_Tx.sem_data_done = osSemaphoreNew(1, 0, NULL);

    drvuart_Rx.mutex_lock_task = osMutexNew(NULL);

    drvuart_Rx.sem_data_done = osSemaphoreNew(1, 0, NULL);
  
    if (HAL_UART_RegisterCallback(drvuart_handle, HAL_UART_TX_COMPLETE_CB_ID, drvuart_TxCpltCallback) != HAL_OK) status = DRVUART_ERROR;
    if (HAL_UART_RegisterRxEventCallback(drvuart_handle, drvuart_RxEventCallback) != HAL_OK) status = DRVUART_ERROR;
    if (HAL_UART_RegisterCallback(drvuart_handle, HAL_UART_ERROR_CB_ID, drvuart_ErrorCallback) != HAL_OK) status = DRVUART_ERROR;

    if (drvuart_CheckHealth() != DRVUART_OK) status = DRVUART_ERROR;
    if(status != DRVUART_OK) drvuart_DeInit(); // Clean up if initialization failed

    return status;
}

drvuart_Status drvuart_DeInit(void)
{
    drvuart_Status status = DRVUART_OK;
    osStatus_t stat = osOK;

    if (drvuart_Tx.mutex_lock_task != NULL) {
        stat = osMutexDelete(drvuart_Tx.mutex_lock_task);
        drvuart_Tx.mutex_lock_task = NULL;
    }

    if (drvuart_Tx.sem_data_done != NULL) {
        stat = osSemaphoreDelete(drvuart_Tx.sem_data_done);
        drvuart_Tx.sem_data_done = NULL;
    }

    if (drvuart_Rx.mutex_lock_task != NULL) {
        stat = osMutexDelete(drvuart_Rx.mutex_lock_task);
        drvuart_Rx.mutex_lock_task = NULL;
    }

    if (drvuart_Rx.sem_data_done != NULL) {
        stat = osSemaphoreDelete(drvuart_Rx.sem_data_done);
        drvuart_Rx.sem_data_done = NULL;
    }

    if(drvuart_handle != NULL) {
        HAL_UART_UnRegisterCallback(drvuart_handle, HAL_UART_TX_COMPLETE_CB_ID);
        HAL_UART_UnRegisterRxEventCallback(drvuart_handle);
        HAL_UART_UnRegisterCallback(drvuart_handle, HAL_UART_ERROR_CB_ID);
        drvuart_handle = NULL;
    }

    if (stat != osOK) status = DRVUART_ERROR;

    return status;
}

drvuart_Status drvuart_CheckHealth(void)
{
    if (drvuart_handle                  == NULL                     || 
        drvuart_Tx.mutex_lock_task      == NULL                     || 
        drvuart_Tx.sem_data_done        == NULL                     ||
        drvuart_Rx.mutex_lock_task      == NULL                     || 
        drvuart_Rx.sem_data_done        == NULL                     ||
        drvuart_handle->TxCpltCallback  != drvuart_TxCpltCallback   ||
        drvuart_handle->RxEventCallback != drvuart_RxEventCallback  ||   
        drvuart_handle->ErrorCallback   != drvuart_ErrorCallback        ) {
        return DRVUART_ERROR;
    }
    return DRVUART_OK;
}

drvuart_Status drvuart_Transmit(const uint8_t *data, uint16_t length, uint32_t timeout)
{
    drvuart_Status status = DRVUART_OK;
    if (drvuart_CheckHealth() != DRVUART_OK) return DRVUART_ERROR;
    if (data == NULL || length == 0) return DRVUART_ERROR;
    if (length > MAX_DATA_LENGTH) return DRVUART_ERROR;

    if(osMutexAcquire(drvuart_Tx.mutex_lock_task, timeout) != osOK) return DRVUART_BUSY;

    // Đảm bảo rằng không còn dữ liệu nào đang chờ xử lý
    while (osSemaphoreAcquire(drvuart_Tx.sem_data_done, 0) == osOK);
    
    memcpy(drvuart_Tx.data, data, length);
    drvuart_Tx.length = length;
    
    if(HAL_UART_Transmit_DMA(drvuart_handle, drvuart_Tx.data, drvuart_Tx.length) != HAL_OK) status = DRVUART_ERROR;
    else if(osSemaphoreAcquire(drvuart_Tx.sem_data_done, timeout) != osOK) status = DRVUART_ERROR;

    

    if (status != DRVUART_OK) {
        // Nếu có lỗi xảy ra, hủy việc truyền dữ liệu
        HAL_UART_AbortTransmit(drvuart_handle);
    }
    osMutexRelease(drvuart_Tx.mutex_lock_task);
    return status;
}
drvuart_Status drvuart_Receive(uint8_t *data, uint16_t *length, uint32_t timeout)
{
    
    drvuart_Status status = DRVUART_OK;

    if (drvuart_CheckHealth() != DRVUART_OK) return DRVUART_ERROR;
    if (data == NULL || length == NULL) return DRVUART_ERROR;

    if(osMutexAcquire(drvuart_Rx.mutex_lock_task, timeout) != osOK) return DRVUART_BUSY;

    // Đảm bảo rằng không còn dữ liệu nào đang chờ xử lý
    while (osSemaphoreAcquire(drvuart_Rx.sem_data_done, 0) == osOK);
    
    // Bắt đầu nhận dữ liệu bằng DMA
    if(HAL_UARTEx_ReceiveToIdle_DMA(drvuart_handle, drvuart_Rx.data, MAX_DATA_LENGTH) != HAL_OK) status = DRVUART_ERROR;
    else if(osSemaphoreAcquire(drvuart_Rx.sem_data_done, timeout) != osOK) status = DRVUART_ERROR;
        else
        {
             memcpy(data, drvuart_Rx.data, drvuart_Rx.length);
            *length = drvuart_Rx.length;
        }
    

    if (status != DRVUART_OK) {
        // Nếu có lỗi xảy ra, hủy việc nhận dữ liệu
        HAL_UART_AbortReceive(drvuart_handle);
    }
    osMutexRelease(drvuart_Rx.mutex_lock_task);
    return status;
}

static void drvuart_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == drvuart_handle) {
        // Nhả Semaphore từ ngắt, đánh thức Task dậy
        osSemaphoreRelease(drvuart_Tx.sem_data_done);
        
    }
}

static void drvuart_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == drvuart_handle) {
        // Nhả Semaphore từ ngắt, đánh thức Task dậy
        drvuart_Rx.length = Size;
        osSemaphoreRelease(drvuart_Rx.sem_data_done);
    }
}

static void drvuart_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == drvuart_handle) {
        osSemaphoreRelease(drvuart_Tx.sem_data_done);
        osSemaphoreRelease(drvuart_Rx.sem_data_done);
    }
}