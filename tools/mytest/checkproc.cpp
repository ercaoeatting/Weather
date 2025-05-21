#include "_public.h"
#include <csignal>
#include <cstdio>
#include <ctime>
#include <string>
#include <unistd.h>

int main(int argc, char *argv[])
{
    // 程序的帮助。
    if (argc != 2) {
        printf("\n");
        printf("Using:./checkproc logfilename\n");

        printf("Example:/project/tools/bin/procctl 10 /project/tools/bin/checkproc /tmp/log/checkproc.log\n\n");
        printf("Example:procctl 10 checkproc /tmp/log/checkproc.log\n\n");
        printf("本程序用于检查后台服务程序是否超时，如果已超时，就终止它。\n");
        printf("注意：\n");
        printf("  1）本程序由procctl启动，运行周期建议为10秒。\n");
        printf("  2）为了避免被普通用户误杀，本程序应该用root用户启动。\n");
        printf("  3）如果要停止本程序，只能用killall -9 终止。\n\n\n");

        return 0;
    }

    // 1 打开日志文件。
    idc::clogfile log;
    if (log.open(argv[1]) == false) {
        std::cout << "logfile " << argv[1] << " open failed" << std::endl;
        return -1;
    }
    // 2 创建/获取共享内存，键值为SHMKEYP，大小为MAXNUMP个st_procinfo结构体的大小。
    int shmid = 0;
    if ((shmid = shmget((key_t)SHMKEYP, MAXNUMP * sizeof(idc::st_procinfo), 0666 | IPC_CREAT)) == -1) {
        log.write("创建或者获取共享内存{}失败", (key_t)SHMKEYP);
        return -1;
    }
    // 3 将共享内存连接到当前进程的地址空间。
    auto *shm = static_cast<idc::st_procinfo *>(shmat(shmid, nullptr, 0));
    // 4 遍历共享内存中全部的记录，如果进程已超时，终止它。
    for (int i = 0; i < MAXNUMP; i++) {
        if (shm[i].pid == 0) {
            continue;
        }
        log.write("进程id={},进程名称={},超时时间={},最后一次心跳时间={}\n", shm[i].pid, shm[i].pname, shm[i].timeout,
                  shm[i].atime);
        // 处理残留的心跳信息
        int iret = kill(shm[i].pid, 0);
        if (iret == -1) {
            log.write("进程{}(pid = {})已经不存在", shm[i].pname, shm[i].pid);
            shm[i] = idc::st_procinfo();
            continue;
        }

        time_t now = time(0);
        if (now - shm[i].atime < shm[i].timeout) {
            continue;
        }

        // 处理超时进程
        log.write("进程id={}超时，终止它。\n", shm[i].pid);
        idc::st_procinfo shmi0 = shm[i]; // 备份要处理的超时进程的信息，因为下面kill 15之后可能shm[i]被析构重置了
        kill(shmi0.pid, 15); // 终止进程。
                             // sleep(5);
        for (int j = 0; j < 5; j++) {
            sleep(1);
            iret = kill(shmi0.pid, 0);
            if (iret == -1) {
                break; // 进程已经退出
            }
        }
        // 5s之后还在的话，认为15杀不掉，用9强杀
        if (iret == -1) {
            log.write("进程{}(pid = {})已经正常终止\n", shmi0.pname, shmi0.pid);
            continue;
        } else {
            kill(shm[i].pid, 9);
            log.write("强制终止进程{}(pid = {})\n", shmi0.pname, shmi0.pid);
            shm[i] = idc::st_procinfo();
        }
    }
    // 5 把共享内存从当前进程中分离。
    shmdt(shm);
}
