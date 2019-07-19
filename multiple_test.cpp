#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <fstream>
#include <ctime>
#include <regex>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_NUM     256

#define FOR_CMD_OLD "%s -m %d -n %d -l %d -e %s -r 3479 -u dry -w 123456 -W 199803305310 -p 3478 %s | tee %s"

#define FOR_CMD_    "%s -m %d -n %d -I %d -e %s -r 3479 -u dry -w 123456 -W 199803305310 -p 3478 %s | tee %s"

#define USAGE       "Usage: ./test [Flags] [options]\n"                                                                     \
                    "Flags:\n"                                                                                              \
                    "           -h <help>       Show the usage of program 'test'.\n"                                        \
                    "           -L <license>    Show the license.\n"                                                        \
                    "Options:\n"                                                                                            \
                    "           -t <total>      Total number of connection.\n"                                              \
                    "           -m <number>     Number of clients per connection.\n"                                        \
                    "           -n <number>     Number of messages to send (default: 5) per client.\n"                      \
                    "           -I <number>     Message length.\n"                                                          \
                    "           -e <ip>         Peer address.\n"                                                            \
                    "           -A <ip>         TURN server address.\n"                                                     \
                    "           -f <folder>     A new folder to store data (default: yy-mm-dd/).\n"                         \
                    "           -c <execute>    Direction of the executable file (default: ./turnutils_uclient).\n"         \
                    "           -s <sleep>      Rand (10 ~ 20) * s as time of sleep after socket error (default: 10 s).\n"  \
                    "Example:\n"                                                                                            \
                    "           ./test -t 10 -m 20 -n 5 -I 1280 -e 192.168.17.8 -A 192.168.22.72\n"

#define LICENSE     "+=======================+\n"   \
                    "+   Author : DuRuYao    +\n"   \
                    "+   Version: 1.0        +\n"   \
                    "+   Date   : 190710     +\n"   \
                    "+=======================+\n" 

using namespace std;

char * get_time();

char * get_f_time();

void handle_opt(int, char **);

void check_folder();

/* Some global variable. */
bool socket_error = false, connect_ok;
int opt, counter = 0;
char cmd[MAX_NUM], mkdir[MAX_NUM], *start_t, temp[64];
float rate, tot_rate = 0, ave_rate = 0;
string long_str = "", rate_str = "", output_str = "";

/* Default options.*/
int t = 1;                              // Total number of connection.
int m = 1;                              // Number of clients.
int n = 5;                              // Number of messages to send per client.
int I = 100;                            // 1.28 KB as length of a message.
int s = 10;                             // Rand (30 ~ 60) * s as the time of sleeping after a socker error.
string peer_ip = "192.168.17.8";        // Address of peer.
string serv_ip = "192.168.22.72";       // Address of TURN server.
string cli_dir = "../turnutils_uclient";// The direction of 'turnutils_uclient'.
string folder_dir = get_f_time();       // The direction of folder to store data.

/* Some files which will be used. */
string rate_file_dir = "rate.txt", out_file_dir = "out.txt", result_file_dir = "result.txt", data_2_py_dir = "data_2_py.txt";

/* Using regex to check socket error by scanning 'out.txt'.*/
regex pattern_1(".+Total lost packets.+\\((.*)%\\),.+"), pattern_2(".*Cannot create socket.*");
smatch result_str;

int main(int argc, char **argv) {
    handle_opt(argc, argv);
    check_folder();

    string draw_cmd = "python3 rate_draw.py -p " + folder_dir + data_2_py_dir;
    out_file_dir = folder_dir + out_file_dir;
    rate_file_dir = folder_dir + rate_file_dir;
    result_file_dir = folder_dir + result_file_dir;
    data_2_py_dir = folder_dir + data_2_py_dir;
    sprintf(cmd, FOR_CMD_OLD, cli_dir.c_str(), m, n, I, peer_ip.c_str(), serv_ip.c_str(), out_file_dir.c_str());
    start_t = get_time();

    cout << "[*] start time: " << start_t;
    cout << "[*] goal cmd  : " << cmd << endl;
    cout << "[*] new folder: " << mkdir << endl;
    cout << "[*] new files : " << out_file_dir << "    " << rate_file_dir << "    " << result_file_dir << endl; 
    // exit(0);

    while (t--) {
        connect_ok = true;
        srand((int) time(NULL));
        int rand_num = rand() % (30) + 30 + 1;
        cout << "----------------------------->\n";
        counter++;
        cout << "[*] progress: " << counter  << "/" << counter + t << endl;
        // counter++;

        /* Execute command to run 'turnutils_uclient'. */
        cout << "[*] relay to execute cmd." << endl;
        // system(cmd);
        pid_t pid = fork();
        if (pid < 0) {
            cout << "[*] fork error." << endl;
        } else if (pid == 0) {
            execl("/bin/sh", "sh", "-c", cmd, (char *)0);
            exit(0);
        } else {
            /* If the connection of child process is timeout, kill it and continue. */
            int wait_t = 0;
            while (true) {
                wait_t++;
                pid_t result = waitpid(pid, NULL, WNOHANG);
                if (result == 0 && wait_t >= 60) {
                    connect_ok = false;
                    kill(pid, 0);
                    break;
                } else if (result == pid) {
                    connect_ok = true;
                    break;
                }
                sleep(1);
            }
            // sleep(60);
            if (connect_ok) {
                cout << "[*] cmd has been executed." << endl;
            } else {
                cout << "[*] net connect is not ok." << endl;
                // kill(pid, 0);
                t++;
                counter--;
                continue;
            }
        }
        fstream readfile, writefile;

        /* Read output of turnutils_uclient from '.../out.txt'. */
        readfile.open(out_file_dir, ios::in);

        while (getline(readfile, long_str)) {

            /* Get the packet loss rate by using regex. */
            if (regex_match(long_str, pattern_2)) {
                socket_error = true;
                break;
            }
            if (regex_match(long_str, result_str, pattern_1)) {
                rate_str = result_str[1];
                break;
            }
        }
        readfile.close();

        if (rate_str == "") socket_error = true;

        if (socket_error) {
            t++;
            counter--;
            printf("[*] try to sleep %d s to avoid SOCKET ERROR.\n", s * rand_num);
            sleep(s * rand_num);
            socket_error = false;
            continue;
        }

        /* Write the packet loss rate to '.../rate.txt'. */
        writefile.open(rate_file_dir, ios::out | ios::app);
        sprintf(temp, "%-8.d    %s\n", counter, rate_str.c_str());
        writefile << temp;
        writefile.close();

        /* Turn rate_str to rate, get total rate and average rate. */
        if (rate_str.length() == 0) {
            rate = 0;
            counter--;
        } else {
            rate = stof(rate_str);
            if (counter != 1)
                output_str += " + " + rate_str + " %";
            else
                output_str += rate_str + " %";
            tot_rate += rate;
            ave_rate = tot_rate / counter;

            /* Write result to '.../result.txt'. */
            writefile.open(result_file_dir, ios::out | ios::trunc);
            if (counter != 0) { 
                writefile << cmd << endl << endl;
                writefile << "( " << output_str << " ) / " << counter << " = " << ave_rate << " %" << endl;
            }
            writefile.close();
            
            /* Write rate data to '.../data.txt'. */
            writefile.open(data_2_py_dir, ios::out | ios::app);
            if (counter != 0) { 
                writefile << rate_str << endl;
            }
            writefile.close();
        }

        rate_str = "";
        char *now_t = get_time();
        cout << "[*] start time  : " << start_t;
        cout << "[*] now   time  : " << now_t; 
	    cout << "[*] average rate: " << ave_rate << "%" << endl;
	    
        printf("[*] sleep %d s.\n", rand_num);
        sleep(rand_num);
    }
    /* Draw and save rate image. */
	cout << "[*] " << draw_cmd << endl;
	system(draw_cmd.c_str());
    return 0;
}

char * get_time() {
    time_t t = time(NULL);
    return ctime(&t);
}

char * get_f_time() {
    time_t timep;
    time (&timep);
    strftime(temp, sizeof(temp), "%y-%m-%d", localtime(&timep));
    return temp;
}

void handle_opt(int argc, char **argv) {
    if (argc <= 1 || argc > 21) {
        cout << "[*] opt error.\n";
        cout << USAGE;
        exit(0);
    }

    while ((opt = getopt(argc, argv, "hLt:m:n:I:e:A:f:c:s:")) != -1) {
        switch (opt) {
            case 'h':
                cout << USAGE;
                exit(0);
                break;
            case 'L':
                cout << LICENSE;
                exit(0);
                break;
            case 't':
                t = stoi(optarg);
                break;
            case 'm':
                m = stoi(optarg);
                break;
            case 'n':
                n = stoi(optarg);
                break;
            case 'I':
                I = stoi(optarg);
                break;
            case 'e':
                peer_ip = optarg;
                break;
            case 'A':
                serv_ip = optarg;
                break;
            case 'f':
                /* '.../19-07-10/' ---> '.../19-07-10' */
                if (optarg[(sizeof(optarg) / sizeof(optarg[0])) - 1] == '/')
                    optarg[(sizeof(optarg) / sizeof(optarg[0])) - 1] = '\0';
                folder_dir = optarg;
                break;
            case 'c':
                /* '.../turnutils_uclient' ---> './.../turnutils_uclient' */
                cli_dir = "./" + (cli_dir = optarg);
                break;
            case 's':
                s = stoi(optarg);
                break;
            default:;
        }
    }
}

void check_folder() {
    string dir = folder_dir + "/";
    for (int count = 1; opendir(dir.c_str()); dir = folder_dir + "_" + to_string(count++) + "_/");
    folder_dir = dir;
    sprintf(mkdir, "mkdir %s", folder_dir.c_str());
    system(mkdir);
}
