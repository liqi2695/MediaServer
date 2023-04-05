#include <string.h>
#include <stdio.h>
 #include <iostream>

static int handleCmd_SETUP(char* result, int cseq, int clientRtpPort)
{
    sprintf(result, "RTSP/1.0 200 OK\r\n"
        "CSeq: %d\r\n"
        "Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d\r\n"
        "Session: 66334873\r\n"
        "\r\n",
        cseq,
        clientRtpPort,
        clientRtpPort + 1,
        55432,
        55433);

    return 0;
}

int main () {
    char *buf = (char*)malloc(10000);
    buf = "sdfdk";
    handleCmd_SETUP(buf, 2, 8080);
    std::cout << buf << std::endl;
//    char str[80] = "This is - www.runoob.com - website";
//    const char s[2] = "-";
//    char *token;
   
//    /* 获取第一个子字符串 */
//    token = strtok(str, s);
   
//    std::cout << strtok(NULL, s) << std::endl;
//    /* 继续获取其他的子字符串 */
//    while( token != NULL ) {
//       printf( "%s\n", token );
    
//       token = strtok(NULL, s);
//    }
//    std::cout << str <<std::endl;
//    return(0);
}