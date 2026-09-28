#ifndef SETTING_H
#define SETTING_H


//cài đặt các thông số cho log
#define LOG_DEBUG 1                 // Bật/tắt tính năng debug log, nếu bật thì log sẽ in ra thông tin chi tiết hơn
#define SET_LOG_MAX_LENGTH 128      // Độ dài tối đa của một log message
#define SET_LOG_MAX_QUEUE_LENGTH 16 // Số lượng log tối đa trong hàng đợi
#define SET_LOG_TIMEOUT 10          // timeout trong ms khi gửi log ra uart
#define SET_TESTLOG_ON 1            // Bật/tắt tính năng test log
#define SET_SET_LOG_STACK_CHECK 1   // Bật/tắt tính năng kiểm tra dung lượng stack còn trống của task log transmit


//cài đặt các thông số cho drvuart
#define SET_DRVUART_MAX_DATA_LENGTH 256     // Độ dài tối đa của dữ liệu gửi/nhận qua UART
#define SET_DRVUART_MAX_INSTANCES 3         // Số lượng instance tối đa của drvuart, tương ứng với số lượng UART mà bạn muốn sử dụng drvuart



#endif