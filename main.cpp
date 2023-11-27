//#include <iostream>
//#include <modbus.h>
//
//// 定义从站地址和寄存器地址
//#define SLAVE_ADDRESS 1
//#define TEST_ADDRESS_1 100
//
//// 定义一个从站结构体
//struct slave_t {
//    modbus_t *ctx; // modbus上下文
//    uint16_t *tab_reg; // 保持寄存器数组
//    int nb_reg; // 保持寄存器数量
//};
//
//// 创建一个从站对象
//slave_t *create_slave(const char *ip, int port, int nb_reg) {
//    // 分配内存空间
//    slave_t *slave = (slave_t *) malloc(sizeof(slave_t));
//    if (slave == NULL) {
//        fprintf(stderr, "无法分配内存空间\n");
//        return NULL;
//    }
//    // 创建modbus上下文
//    slave->ctx = modbus_new_tcp(ip, port);
//    if (slave->ctx == NULL) {
//        fprintf(stderr, "无法创建modbus上下文\n");
//        free(slave);
//        return NULL;
//    }
//    // 设置从站地址
//    modbus_set_slave(slave->ctx, SLAVE_ADDRESS);
//    // 分配保持寄存器数组
//    slave->nb_reg = nb_reg;
//    slave->tab_reg = (uint16_t *) malloc(nb_reg * sizeof(uint16_t));
//    if (slave->tab_reg == NULL) {
//        fprintf(stderr, "无法分配保持寄存器数组\n");
//        modbus_free(slave->ctx);
//        free(slave);
//        return NULL;
//    }
//    // 初始化保持寄存器数组
//    for (int i = 0; i < nb_reg; i++) {
//        slave->tab_reg[i] = 0;
//    }
//    // 返回从站对象
//    return slave;
//}
//
//// 销毁一个从站对象
//void destroy_slave(slave_t *slave) {
//    // 释放保持寄存器数组
//    free(slave->tab_reg);
//    // 释放modbus上下文
//    modbus_free(slave->ctx);
//    // 释放内存空间
//    free(slave);
//}
//
//// 运行一个从站对象
//void run_slave(slave_t *slave) {
//    // 打开TCP连接
//    if (modbus_connect(slave->ctx) == -1) {
//        fprintf(stderr, "无法打开TCP连接\n");
//        return;
//    }
//    // 循环监听请求
//    while (1) {
//        // 定义一个modbus请求结构体
//        modbus_mapping_t *mb_mapping;
//        // 分配内存空间
//        mb_mapping = modbus_mapping_new(0, 0, slave->nb_reg, 0);
//        if (mb_mapping == NULL) {
//            fprintf(stderr, "无法分配modbus请求结构体\n");
//            break;
//        }
//        // 定义一个请求缓冲区
//        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
//        // 等待接收请求
//        int rc = modbus_receive(slave->ctx, query);
//        if (rc == -1) {
//            // 如果出错，跳过本次循环
//            modbus_mapping_free(mb_mapping);
//            continue;
//        }
//        // 处理请求
//        // 读保持寄存器
//        if (query[7] == MODBUS_FC_READ_HOLDING_REGISTERS) {
//
//            // 将从站的保持寄存器数组复制到请求结构体中
//            for (int i = 0; i < slave->nb_reg; i++) {
//                mb_mapping->tab_registers[i] = slave->tab_reg[i];
//            }
//        }
//            // 写单个保持寄存器
//        else if (query[7] == MODBUS_FC_WRITE_SINGLE_REGISTER) {
//            // 将请求结构体中的保持寄存器值复制到从站的保持寄存器数组中
//            int address = (query[8] << 8) + query[9];
//            int value = (query[10] << 8) + query[11];
//            slave->tab_reg[address] = value;
//            // 如果写入的是TEST_ADDRESS_1，那么设置其值为1，并等待5秒后设置为0
//            if (address == TEST_ADDRESS_1) {
//                slave->tab_reg[address] = 1;
////                sleep(5);
//                slave->tab_reg[address] = 0;
//            }
//        }
//        // 回复请求
//        modbus_reply(slave->ctx, query, rc, mb_mapping);
//        // 释放请求结构体
//        modbus_mapping_free(mb_mapping);
//    }
//
//// 关闭TCP连接
//    modbus_close(slave->ctx);
//}
//
//// 主函数
//int main() {
//    // 创建一个从站对象，IP地址为"127.0.0.1"，端口为502，保持寄存器数量为200
//    slave_t *slave = create_slave("127.0.0.1", 502, 200);
//    if (slave == NULL) {
//        fprintf(stderr, "无法创建从站对象\n");
//        return -1;
//    }
//    // 运行从站对象
//    run_slave(slave);
//    // 销毁从站对象
//    destroy_slave(slave);
//    return 0;
//}



#include <stdio.h>
#include <iostream>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <thread> // 引入多线程头文件
#include <mutex> // 引入互斥锁头文件
#include <condition_variable> // 引入条件变量头文件

#include <modbus.h>

#if defined(_WIN32)
#define close closesocket
#endif

// 定义一些地址常量
#define CLIENT_COUNTS_ADDRESS 0
#define FLAG_PROCESS_START_ADDRESS 1
#define FLAG_ADDRESS_1 2

// 定义一个全局变量，用于判断是否有连接
bool connected = false;
bool closed = false;

// 定义一个互斥锁，用于保护条件变量
std::mutex mtx;

// 定义一个条件变量，用于等待和通知连接状态
std::condition_variable cv;

// 定义一个线程函数，用于修改标志位
void flag_thread(modbus_mapping_t *mb_mapping) {
    // 使用互斥锁和条件变量来等待连接成功
    std::unique_lock<std::mutex> lock(mtx); // 上锁
    cv.wait(lock, [&] { return connected || closed; }); // 等待条件变量满足
    lock.unlock(); // 解锁
    // 判断是否关闭
    if (closed) {
        // 退出线程
        return;
    }
    std::cout << "开始处理";

    // 将FLAG_PROCESS_START_ADDRESS改为1
    mb_mapping->tab_registers[FLAG_PROCESS_START_ADDRESS] = 1;
    // 过3秒，将FLAG_ADDRESS_1改为1
    std::this_thread::sleep_for(std::chrono::seconds(3));
    mb_mapping->tab_registers[FLAG_ADDRESS_1] = 1;
    std::cout << "数据1";

    // 再过3秒后，再次将FLAG_ADDRESS_1改为1
    std::this_thread::sleep_for(std::chrono::seconds(3));
    mb_mapping->tab_registers[FLAG_ADDRESS_1] = 1;
    std::cout << "数据2";

    mb_mapping->tab_registers[FLAG_PROCESS_START_ADDRESS] = 0;
}

int main(void) {
    int s = -1;
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping;

    ctx = modbus_new_tcp("127.0.0.1", 502);
    /* modbus_set_debug(ctx, TRUE); */

    mb_mapping = modbus_mapping_new(500, 500, 500, 500);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    s = modbus_tcp_listen(ctx, 1);
    modbus_tcp_accept(ctx, &s);

    // 创建一个线程，传入线程函数和映射参数
    std::thread t(flag_thread, mb_mapping);

    for (;;) {
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
        int rc;

        rc = modbus_receive(ctx, query);
        if (rc > 0) {
            // rc is the query size
            // 在有连接的时候，CLIENT_COUNTS_ADDRESS能够自动+1
            if (!connected) {
                connected = true;
                mb_mapping->tab_registers[CLIENT_COUNTS_ADDRESS]++;

                printf("打开连接");
                std::cout<<"打开连接";
                // 通知条件变量已经改变
                cv.notify_one();
            }
            modbus_reply(ctx, query, rc, mb_mapping);
        } else if (rc == -1) {
            // Connection closed by the client or error

            closed = true;
            cv.notify_one();
            break;
        }
    }

    printf("Quit the loop: %s\n", modbus_strerror(errno));

    if (s != -1) {
        close(s);
    }
    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);

    // 等待线程结束
    t.join();

    return 0;
}
