#include "_public.h"
#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

struct procinfo {
    int pid = 0;            // 进程ID 主键
    char name[60];          // 进程名称
    int timeout = 0;        // 超时时间
    time_t last_heart_time; // 最后一次心跳的时间
    procinfo() = default;

    procinfo(int const pid, std::string const &in_name, int const timeout, int const lasttime)
        : pid(pid),
          timeout(timeout),
          last_heart_time(lasttime)
    {
        strncpy(name, in_name.c_str(), 60);
    };
};

int shmid = -1;               // 共享内存ID
procinfo *shm_proc = nullptr; // 指向共享内存的地址空间
void EXIT(int sig);
int pos = -1;

int main(int argc, char **argv)
{
    // 1 处理程序的退出信号。
    signal(SIGINT, EXIT);
    // 2 创建/获取共享内存。
    shmid = shmget(0x5005, 1000 * sizeof(procinfo), 0640 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    std::cout << "shmid = " << shmid << std::endl;
    // 3 将共享内存连接到当前进程的地址空间。
    shm_proc = (procinfo *)shmat(shmid, 0, 0);
    // test print
    for (int i = 0; i < 1000; i++) {
        if (shm_proc[i].pid != 0) {
            printf("i = %d,pid = %d,name = %s,timeout = %d,lasttime = %ld\n", i, shm_proc[i].pid, shm_proc[i].name,
                   shm_proc[i].timeout, shm_proc[i].last_heart_time);
        }
    }
    // 4 把当前进程的信息填充到结构体中。
    procinfo serverinfo(getpid(), argv[0], 30, time(0));
    idc::csemp semlock; // 信号量，用于给共享内存加锁
    if (semlock.init(0x5095) == false) {
        cout << "创建/获取信号量失败" << endl;
        return -1;
    }
    semlock.wait();
    // 5 在共享内存中寻找一个空的位置，把当前进程的结构体保存到共享内存中。
    // 先看有没有残留的编号
    for (int i = 0; i < 1000; i++) {
        if ((shm_proc + i)->pid == getpid()) {
            pos = i;
            cout << "找到了残留的位置" << i << endl;
            break;
        }
    }
    if (pos == -1) {
        for (int i = 0; i < 1000; i++) {
            if ((shm_proc + i)->pid == 0) {
                pos = i;
                cout << "找到了新位置" << i << endl;
                break;
            }
        }
    }
    if (pos == -1) {
        semlock.post();
        cout << "共享内存空间已经用完";
        return -1;
    }
    // assert(pos != -1);
    // memcpy(shm_proc + pos, &serverinfo, sizeof(procinfo));
    // new (shm_proc + pos) procinfo(serverinfo);
    shm_proc[pos] = serverinfo;
    semlock.post();
    while (1) {
        std::cout << "服务程序正在运行中\n";
        // 更新进程的心跳信息
        sleep(25);
        shm_proc[pos].last_heart_time = time(0);
        sleep(25);
    }

    return 0;
}

void EXIT(int sig)
{
    cout << "sig = " << sig;
    // 从共享内存中删除当前进程的心跳信息
    if (pos != -1) {
        // (shm_proc + pos)->~procinfo(); 不对，对于POD类型什么也不会做
        memset(shm_proc + pos, 0, sizeof(procinfo));
    }
    // 把共享内存从当前进程分离
    if (shm_proc != nullptr && shm_proc != (void *)-1) {
        shmdt(shm_proc);
    }
    exit(0);
}
