#include "_public.h"
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <list>
#include <string>
#include <unistd.h>

using namespace std;
using namespace idc;

// 省   站号  站名 纬度   经度  海拔高度
// 安徽,58015,砀山,34.27,116.2,44.2
// 安徽,58102,亳州,33.47,115.44,39.1
struct st_stcode       // 站点参数
{
    char provname[31]; // 省
    char obtid[11];    // 站号
    char obtname[31];  // 站名
    double lat;        // 纬度
    double lon;        // 经度
    double height;     // 海拔高度 m
};

list<st_stcode> siteDatas;              // 存放站点数据
bool load_stcode(string const &infile); // 加载站点数据函数
void crtsurfdata();                     // 根据st_list的站点参数生成站点观测数据存在data_list
bool crtsurffile(string const &out_path, string const &datafmt); // 把data_list中的数据写入文件

struct st_surfdata {
    char obtid[11];     // 站点代码
    char ddatetime[15]; // 数据时间：格式yyyymmddhh24miss,精确到分钟,秒固定填00。
    int t;              // 气温：单位,0.1摄氏度。
    int p;              // 气压：0.1百帕。
    int u;              // 相对湿度,0-100之间的值。
    int wd;             // 风向,0-360之间的值。
    int wf;             // 风速：单位0.1m/s
    int r;              // 降雨量：0.1mm。
    int vis;            // 能见度：0.1米。
};

char str_ddatetime[15];          // 全局变量的当前时间
list<st_surfdata> observedDatas; // 存放站点的观测数据
clogfile logfile;

void EXIT(int signal)
{
    logfile.write("exit because %d\n", signal);
    exit(0);
}

cpactive pactive; // 进程心跳，用全局对象（保证析构函数会被调用）。

int main(int argc, char *argv[])
{
    if (argc != 5) {
        // 如果参数非法,给出帮助文档。
        cout << "Using:./crtsurfdata inifile outpath logfile datafmt\n";
        cout << "Examples:/home/han/CPP/Weather/tools/bin/procctl 60 /home/han/CPP/Weather/idc/cpp/crtsurfdata "
                "/home/han/CPP/Weather/idc/ini/stcode.ini "
                "/tmp/idc/surfdata /log/idc/crtsurfdata.log csv,xml,json\n\n";

        cout << "本程序用于生成气象站点观测的分钟数据,程序每分钟运行一次,由调"
                "度模块启动。\n";
        cout << "inifile  气象站点参数文件名。\n";
        cout << "outpath  气象站点数据文件存放的目录。\n";
        cout << "logfile  本程序运行的日志文件名。\n";
        cout << "datafmt  "
                "输出数据文件的格式,支持csv、xml和json,中间用逗号分隔。\n\n";

        return -1;
    }
    closeioandsignal(true);
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);
    pactive.addpinfo(10, "crtsurfdata"); // 把当前进程的信息加入共享内存进程组中。

    if (logfile.open(argv[3]) == false) {
        return -1;
    }
    logfile.write("start run crtsurf\n");
    // 1）从站点参数文件中加载站点参数,存放于容器中；
    if (!load_stcode(argv[1])) {
        EXIT(-1);
    }
    // 2）根据站点参数,生成站点观测数据（随机数）；
    memset(str_ddatetime, 0, sizeof(str_ddatetime));
    ltime(str_ddatetime, "yyyymmddhh24miss");
    str_ddatetime[12] = str_ddatetime[13] = '0';
    crtsurfdata();
    // 3）把站点观测数据存保到文件中。
    if (strstr(argv[4], "csv") != 0) {
        crtsurffile(argv[2], "csv");
    }
    if (strstr(argv[4], "xml") != 0) {
        crtsurffile(argv[2], "xml");
    }
    if (strstr(argv[4], "json") != 0) {
        crtsurffile(argv[2], "json");
    }
    logfile.write("stop run crtsurf\n");
}

bool load_stcode(string const &infile)
{
    cifile iflie; // 读取文件的对象
    if (!iflie.open(infile)) {
        logfile.write("open %s failed\n", infile.c_str());
        return false;
    }
    string line;
    iflie.readline(line); // 不处理标题
    ccmdstr cmdstr;
    st_stcode stcode;
    while (iflie.readline(line)) {
        // logfile.write("写入了站点数据  %s\n", line.c_str());
        cmdstr.splittocmd(line, ",");
        memset(&stcode, 0, sizeof(stcode));
        cmdstr.getvalue(0, stcode.provname);
        cmdstr.getvalue(1, stcode.obtid);
        cmdstr.getvalue(2, stcode.obtname);
        cmdstr.getvalue(3, stcode.lat);
        cmdstr.getvalue(4, stcode.lon);
        cmdstr.getvalue(5, stcode.height);
        siteDatas.push_back(stcode);
    }
    // for (auto &data: st_list) {
    //     logfile.write(
    //         "省份 %s 站号 %s 站名 %s 经纬度(%lf,%lf) 海拔高度 %lf m\n",
    //         data.provname, data.obtid, data.obtname, data.lat, data.lon,
    //         data.height);
    // }
    return true;
}

void crtsurfdata()
{
    srand(time(0));
    st_surfdata surfdata;
    for (auto &data: siteDatas) {
        memset(&surfdata, 0, sizeof(st_surfdata));
        strcpy(surfdata.obtid, data.obtid);
        strcpy(surfdata.ddatetime, str_ddatetime);
        surfdata.t = rand() % 350; // 气温：单位,0.1摄氏度。0-350之间。可犯可不犯的错误不要犯。
        surfdata.p = rand() % 265 + 10000;     // 气压：0 .1百帕
        surfdata.u = rand() % 101;             // 相对湿度,0-100之间的值。
        surfdata.wd = rand() % 360;            // 风向,0-360之间的值。
        surfdata.wf = rand() % 150;            // 风速：单位0.1m/s。
        surfdata.r = rand() % 16;              // 降雨>量：0.1mm。
        surfdata.vis = rand() % 5001 + 100000; // 能见度：0.1米。
        observedDatas.push_back(surfdata);
    }
    // for (auto &data: data_list) {
    //     logfile.write("%s,%s,%.1f%.1f,%d,%d,%.1f,%.1f%.1f\n", data.obtid, data.ddatetime, data.t / 10.0, data.p
    //     / 10.0, data.u,
    //                   data.wd, data.wf / 10.0, data.r / 10.0, data.vis / 10.0);
    // }
}

bool crtsurffile(string const &out_path, string const &datafmt) // 把data_list中的数据写入文件
{
    string file_name = out_path + "/" + "SURF_ZH_" + str_ddatetime + "_" + to_string(getpid()) + "." + datafmt;
    cofile outfile;
    if (outfile.open(file_name) == false) {
        logfile.write("open %s failed\n", file_name.c_str());
        return false;
    }
    if (datafmt == "csv") {
        outfile.writeline("站点代码,数据时间,气温,气压,相对湿度,风向,风速,降雨量,能见度\n");
    }
    if (datafmt == "xml") {
        outfile.writeline("<data>\n");
    }
    if (datafmt == "json") {
        outfile.writeline("{\"data\":[\n");
    }
    for (auto &data: observedDatas) {
        if (datafmt == "csv") {
            outfile.writeline("%s,%s,%.1f,%.1f,%d,%d,%.1f,%.1f,%.1f\n", data.obtid, data.ddatetime, data.t / 10.0,
                              data.p / 10.0, data.u, data.wd, data.wf / 10.0, data.r / 10.0, data.vis / 10.0);
        }
        if (datafmt == "xml") {
            outfile.writeline("<obtid>%s</obtid>,<ddatetime>%s</ddatetime>,<t>%.1f</t>,<p>%.1f</p>,<u>%d</u>,<wd>%d</"
                              "wd>,<wf>%.1f</wf>,<r>%.1f</r>,<vis>%.1f</vis></endl>\n",
                              data.obtid, data.ddatetime, data.t / 10.0, data.p / 10.0, data.u, data.wd, data.wf / 10.0,
                              data.r / 10.0, data.vis / 10.0);
        }
        if (datafmt == "json") {
            outfile.writeline("{\"obtid\": \"%s\","
                              "\"ddatetime\": \"%s\","
                              "\"t\": \"%.1f\","
                              "\"p\": \"%.1f\","
                              "\"u\": \"%d\","
                              "\"wd\": \"%d\","
                              "\"wf\": \"%d\","
                              "\"vis\": \"%.1f\"}",
                              data.obtid, data.ddatetime, data.t / 10.0, data.p / 10.0, data.u, data.wd, data.wf / 10.0,
                              data.r / 10.0, data.vis / 10.0);
            static int ii = 0;
            if (ii < observedDatas.size() - 1) {
                outfile.writeline(",\n");
                ii++;
            } else {
                outfile.writeline("\n");
            }
        }
    }
    if (datafmt == "xml") {
        outfile.writeline("</data>\n");
    }
    if (datafmt == "json") {
        outfile.writeline("]}\n");
    }
    outfile.closeandrename();
    logfile.write("生成数据文件 %s 成功，数据时间 %s ，记录数 %d\n", file_name.c_str(), str_ddatetime,
                  observedDatas.size());
    return true;
}
