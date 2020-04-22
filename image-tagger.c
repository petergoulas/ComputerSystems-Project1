/*
** image--tagger.c Latest Version Friday 5-5-19 11pm Panagiotis (Peter) Goulas - Commputer Systems - Assignment 1- final final
*/ 

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// constants
static char const * const HTTP_200_FORMAT = "HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Content-Length: %ld\r\n\r\n";
static char const * const HTTP_400 = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
static int const HTTP_400_LENGTH = 47;
static char const * const HTTP_404 = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
static int const HTTP_404_LENGTH = 45;

// kept global values because when i put them inside program doesnt work correctly
int count =0, endgame_counter2 = 0, start_again = 0, start_again2 = 0;
int guesses_counter = 0, guesses_counter2 = 0, endgame = 0,socket4_trigger = 0,socket5_trigger = 0;
int start_condition = 0, start_condition2 = 0, discarded_condition2 = 0,discarded_condition = 0, endgame_counter = 0;
char guesses1[100][100], guesses2[100][100];




// represents the types of method
typedef enum
{
    GET,
    POST,
    UNKNOWN
} METHOD;

// function prototypes here
bool username_guesses_appear(char* buff, char* userName, int sockfd, int size, int guesses_stage, int guesses_counter, int guesses_counter2);


static bool handle_http_request(int sockfd)
{
    // try to read the request
    char buff[2049], mid[2049], result[6149];
    int n = read(sockfd, buff, 2049),word_count4 = 0, word_count5 =0, guesses_stage = 0;
    int condition = 0, size_curr = 0, quit_condition = 0, quit_condition2 = 0, general_cond = 0, general_cond2 =0;
    if (n <= 0)
    {
        if (n < 0)
            perror("read");
        else
            printf("socket %d close the connection\n", sockfd);
        return false;
    }

    // terminate the string
    buff[n] = 0;

    char * curr = buff;

    // parse the method
    METHOD method = UNKNOWN;
    if (strncmp(curr, "GET ", 4) == 0)
    {
        curr += 4;
        method = GET;
    }
    else if (strncmp(curr, "POST ", 5) == 0)
    {
        curr += 5;
        method = POST;
    }
    else if (write(sockfd, HTTP_400, HTTP_400_LENGTH) < 0)
    {
        perror("write");
        return false;
    }

    // sanitise the URI
    while (*curr == '.' || *curr == '/')
        ++curr;
    
    size_curr = strlen(curr);

    // assume the only valid request URI is "/" but it can be modified to accept more files
    if (size_curr > 0) {

        if ((method == GET) && (quit_condition == 0) && (quit_condition2 == 0)) 
        {
            /* check if we are using a GET or a POST method and if not use fprint for error message */
            if (sockfd == 4) {
                general_cond = 1;
            } else {
                general_cond2 = 1;
            }
            if (strstr(buff, "start")!= NULL) {
                condition = 1;
            }

            // get the size of the file
            struct stat st;
            if (condition == 0) {
                stat("1_intro.html", &st);


            }
            if (condition == 1) {
                stat("3_first_turn.html", &st);

                if ((sockfd == 4) && (endgame == 0)){
                    start_condition = 1;
                } 
                if ((sockfd == 5) && (endgame == 0)){
                    start_condition2 = 1;
                }

                // endgame conditions
                if ((sockfd == 4) && (endgame == 1) && (start_again2 >= 1) && (start_again >=1)) {
                    
                    endgame = 0;
                    start_again = 0;
                    start_again2 = 0;
                    start_condition = 1;
                    //start_condition2 = 1;
                }
                if ((sockfd == 5) && (endgame == 1) && (start_again >= 1) && (start_again >=1)) {
                    
                    endgame = 0;
                    start_again = 0;
                    start_again2 = 0;
                    //start_condition = 1;
                    start_condition2 = 1;
                }

            }
            n = sprintf(buff, HTTP_200_FORMAT, st.st_size);
            // send the header first
            if (write(sockfd, buff, n) < 0)
            {
                perror("write");
                return false;
            }

            // send the file
            int filefd; 
            // get request open intro if condition satisfied else no 3
            if (condition == 0) {
                filefd = open("1_intro.html", O_RDONLY);
            }
            if (condition == 1) {
                filefd = open("3_first_turn.html", O_RDONLY);
            }
            do
            {
                n = sendfile(sockfd, filefd, NULL, 2048);
            }
            while (n > 0);
            if (n < 0)
            {
                perror("sendfile");
                close(filefd);
                return false;
            }
            close(filefd);
        // quit if a player quits 
        } else if ((method == POST) || ((method == GET) && (quit_condition == 1)) || ((method == GET) && (quit_condition2 ==1)))
        {
            /* check if we are using a GET or a POST method and if not use fprint for error message */
            if (sockfd == 4) {
                general_cond = 1;
            } else {
                general_cond2 = 1;
            }
            
            // get the size of the file
            struct stat st;
            // quit section loading page
            if (((strstr(buff, "quit=Quit"))!= NULL) || ((method == GET) && (quit_condition == 1)) || ((method == GET) && (quit_condition2 ==1)) 
                || (((method == POST) && (quit_condition == 1))) || ((method == POST) && (quit_condition2 == 1)) ) 
            {

                quit_condition = 1;
                quit_condition2 = 1;

                stat("7_gameover.html", &st);
                long size = st.st_size;
                n = sprintf(buff, HTTP_200_FORMAT, size);
                // send the header first
                if (write(sockfd, buff, n) < 0)
                {
                    perror("write");
                    return false;
                }
                // read the content of the HTML file
                // int filefd = open("2_start.html", O_RDONLY);
                int filefd;
                filefd = open("7_gameover.html", O_RDONLY);
                
                n = read(filefd, buff, 2048);
                if (n < 0)
                {
                    perror("read");
                    close(filefd);
                    return false;
                }
                close(filefd);

                if (write(sockfd, buff, size) < 0)
                {
                    perror("write");
                    return false;
                }
            } else {
                /* check if we are using a GET or a POST method and if not use fprint for error message */
                if (sockfd == 4) {
                    general_cond = 1;
                } else {
                    general_cond2 = 1;
                }
                // pressing the guess button triggers it
                if (((strstr(buff, "guess=Guess"))!= NULL))
                {
                    guesses_stage = 1;
                    char word[100];
                    char word2[100];
                    int i = 0, j = 0, k = 0, m = 0;

                    // save the word
                    if ((start_condition == 1) && (start_condition2 == 1)){
                        if (sockfd == 4) {
                            char tempword[1000];
                            //char word[100];
                            strcpy (word, ((strstr(buff, "keyword=") + 8)) );

                            int len = strlen(word);
                            word[len-12] = '\0';
  
                            strcpy(guesses1[guesses_counter], word);
   
                            if (guesses_counter2 >= 1){
                                for (m = 0; m <= (guesses_counter2-1); m++) {
                                    if (strcmp(word, guesses2[m]) == 0){
                                        endgame = 1;
                                        start_condition2 = 0;
                                        start_condition = 0;
                                    }
                                }
                            }
                            guesses_counter += 1;

                        }
                    }
                    // compare the word and if its same endgame
                    if ((start_condition == 1) && (start_condition2 == 1)){
                        if (sockfd == 5) {
                            strcpy (word2, ((strstr(buff, "keyword=") + 8)) );
                            int len2 = strlen(word2);
                            word2[len2-12] = '\0';

                            strcpy(guesses2[guesses_counter2], word2);

                            if (guesses_counter == 1){
                                if (strcmp(word2, word) == 0){
                                    endgame = 1;
                                    start_condition = 0;
                                    start_condition2 = 0;
                                }
                            } else {
                                for (k = 0; k <= (guesses_counter-1); k++ ){
                                    if (strcmp(word2, guesses1[k]) == 0){
                                        endgame = 1;
                                        start_condition = 0;
                                        start_condition2 = 0;
                                    }
                                }
                            }
                            guesses_counter2 += 1;
                        }
                    }
                    // accepted when both players can start else discarded
                    if (endgame == 0) {
                        if ((start_condition == 1) && (start_condition2 == 1)){
                            if (sockfd == 4){
                                word_count4 += 1;

                            } else {
                                word_count5 +=1;
                            }
                            stat("4_accepted.html", &st);
                            discarded_condition = 0;
                        } else {

                            memset(guesses1, 0, sizeof(guesses1));
                            memset(guesses2, 0, sizeof(guesses2));
                            stat("5_discarded.html", &st);
                            discarded_condition = 1;
                            discarded_condition2 = 1;

                        }

                    } else {
                        // clean arrays of guesses
                        if ( ((sockfd == 4) && (start_condition2 == 0) &&(start_again == 0)) || ((sockfd == 5) && (start_condition == 0) && (start_again2 == 0)) ) {
                            memset(guesses1, 0, sizeof(guesses1));
                            memset(guesses2, 0, sizeof(guesses2));
                            guesses_counter = 0;
                            guesses_counter2 = 0;
                            if (sockfd == 4){
                                start_again += 1;
                            }
                            if (sockfd == 5) {
                                start_again2 += 1;
                            }
                            stat("6_endgame.html", &st);

                        } else {
                            stat("5_discarded.html", &st);    
                        }

                    }
                    long size = st.st_size;
                    n = sprintf(buff, HTTP_200_FORMAT, size);
                    // send the header first
                    if (write(sockfd, buff, n) < 0)
                    {
                        perror("write");
                        return false;
                    }
                    // read the content of the HTML file
                    // int filefd = open("2_start.html", O_RDONLY);
                    int filefd;
                    if (endgame == 0) {
                        if ((start_condition == 1) && (start_condition2 == 1)){
                            filefd = open("4_accepted.html", O_RDONLY);
                        } else {
                            filefd = open("5_discarded.html", O_RDONLY);                            

                        }
                    // only with these conditions player can end game
                    } else {
                        if (((sockfd == 4) && (start_condition2 == 0)) || ((sockfd == 5) && (start_condition == 0))) {
                            filefd = open("6_endgame.html", O_RDONLY);
                            if (sockfd == 5){
                            }
                        } else {
                            filefd = open("5_discarded.html", O_RDONLY);
                        }
                    }
                    
                    n = read(filefd, buff, 2048);
                    if (n < 0)
                    {
                        perror("read");
                        close(filefd);
                        return false;
                    }
                    close(filefd);

                    // make guesses appear on page
                    if ((start_condition == 1) && (start_condition2 == 1) && (endgame == 0)) {
                        if (sockfd == 4){
                            username_guesses_appear(buff,word,sockfd,size, guesses_stage, guesses_counter, guesses_counter2);
                        } else {
                            username_guesses_appear(buff,word2,sockfd,size, guesses_stage, guesses_counter, guesses_counter2);
                        }
                    } else {
                            if (write(sockfd, buff, size) < 0) {
                                    perror("write");
                                    return false;
                            } 
                     }

                    if (sockfd == 4){
                        if (endgame == 1){
                            if ((start_condition2 == 1)) {
                                
                            }
                            guesses_counter = 0;
                            start_condition = 0;

                        }
                    }
                    if (sockfd == 5) {
                        if (endgame == 1){
                            if ((start_condition == 1)) {
                                
                            }
                            guesses_counter2 = 0;
                            start_condition2 = 0;

                        }
                    }
                } else {

                    // start condition

                    char * username = strstr(buff, "user=") + 5;
                    int username_length = strlen(username);
                    char userName[50];

                    // the length needs to include the ", " before the username
                    long added_length = username_length + 2;


                    strcpy(userName, username);

                    stat("2_start.html", &st);
                
                    // increase file size to accommodate the username
                    long size = st.st_size + added_length;
                    n = sprintf(buff, HTTP_200_FORMAT, size);
                    // send the header first
                    if (write(sockfd, buff, n) < 0)
                    {
                        perror("write");
                        return false;
                    }
                    // read the content of the HTML file
                    // int filefd = open("2_start.html", O_RDONLY);
                    int filefd;
                    filefd = open("2_start.html", O_RDONLY);
                    
                    n = read(filefd, buff, 2048);
                    if (n < 0)
                    {
                        perror("read");
                        close(filefd);
                        return false;
                    }
                    close(filefd);
                    username_guesses_appear(buff,userName,sockfd,size, guesses_stage, guesses_counter, guesses_counter2);
                }
            }
        }

        if (general_cond2 == 0){
            if (sockfd == 5) {
                fprintf(stderr, "no other methods supported");
            }
        }
        if (general_cond == 0) {
            if (sockfd == 4) {
                fprintf(stderr, "no other methods supported");
            }
        }
    // send 404
    } else if (write(sockfd, HTTP_404, HTTP_404_LENGTH) < 0)
    {
        perror("write");
        return false;
    }

    return true;
}

int main(int argc, char * argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "usage: %s ip port\n", argv[0]);
        return 0;
    }

    // create TCP socket which only accept IPv4
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // reuse the socket if possible
    int const reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // create and initialise address we will listen on
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // if ip parameter is not specified
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    // bind address to socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // listen on the socket
    listen(sockfd, 5);

    // initialise an active file descriptors set
    fd_set masterfds;
    FD_ZERO(&masterfds);
    FD_SET(sockfd, &masterfds);
    // record the maximum socket number
    int maxfd = sockfd;

    while (1)
    {
        // monitor file descriptors
        fd_set readfds = masterfds;
        if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) < 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // loop all possible descriptor
        for (int i = 0; i <= maxfd; ++i)
            // determine if the current file descriptor is active
            if (FD_ISSET(i, &readfds))
            {
                // create new socket if there is new incoming connection request
                if (i == sockfd)
                {
                    struct sockaddr_in cliaddr;
                    socklen_t clilen = sizeof(cliaddr);
                    int newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen);
                    if (newsockfd < 0)
                        perror("accept");
                    else
                    {
                        // add the socket to the set
                        FD_SET(newsockfd, &masterfds);
                        // update the maximum tracker
                        if (newsockfd > maxfd)
                            maxfd = newsockfd;
                        // print out the IP and the socket number
                        char ip[INET_ADDRSTRLEN];
                        printf(
                            "new connection from %s on socket %d\n\n",
                            // convert to human readable string
                            inet_ntop(cliaddr.sin_family, &cliaddr.sin_addr, ip, INET_ADDRSTRLEN),
                            newsockfd
                        );
                        // print out ip and port number
                        printf("image_tagger server is now running at IP: %s on port %d\n",
                            inet_ntop(cliaddr.sin_family, &cliaddr.sin_addr, ip, INET_ADDRSTRLEN), serv_addr.sin_port);

                    }
                }
                // a request is sent from the client
                else if (!handle_http_request(i))
                {
                    close(i);
                    FD_CLR(i, &masterfds);
                }
            }

    }

    return 0;
}

// function to prin usernames and guesses 
//split buffer to 3 places to put the name inside
bool username_guesses_appear(char* buff, char* userName, int sockfd, int size, int guesses_stage, int guesses_counter, int guesses_counter2) {

    int i = 0;

    char tempbuff[2049], tempbuff2[2049];
    // Part |
    char *test = strstr(buff, "<!DOCTYPE");

    int k = 0;
    //Part III
    char *test2 = strstr(buff, "<form");

    char userName2[50];

    int len1 =  strlen(test),len2 = strlen(test2),len_dif = len1- len2, userName_length = 0;

    int len3 = strlen(userName2);

    strcpy(tempbuff, test);

    userName_length = strlen(userName);

    tempbuff[len_dif] = '\0';

    strcpy(tempbuff2, tempbuff);

    if ((guesses_stage == 0)) {
    // Part II
        sprintf(userName2, "<p>%s</p>", userName);

        strcat(tempbuff2, userName2);
    } else {
        if (sockfd == 4){
            if (strcmp(guesses1[0], "") > 0) {
                sprintf(userName2, "<p>%s", guesses1[0]);
                strcat(tempbuff2, userName2);
            }
            for (k = 1; k <= (guesses_counter-1); k++) {
                if (strcmp(guesses1[k], "") > 0) {
                    sprintf(userName2, ",%s", guesses1[k]);
                    strcat(tempbuff2, userName2);
                }
            }
        } else {
            sprintf(userName2, "<p>%s", guesses2[0]);
            strcat(tempbuff2, userName2);
            for (k = 1; k <= (guesses_counter2-1); k++) {
                sprintf(userName2, ",%s", guesses2[k]);
                strcat(tempbuff2, userName2);
            }
        }
    }
    strcat(tempbuff2, test2);

    if (write(sockfd, tempbuff2, size) < 0)
    {
        perror("write");
        return false;
    } 
}









