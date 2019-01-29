
/********************************************************
   Name: Deyonta Robinson
   Date: 11/04/2018
   rpc1.c - Rock, Paper, Scissors game with IPC
   Programming Assigment 03
 *********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#define SOCKET_PATH "RPS_socket"
int main(int argc, char const *argv[]) {

  //convert cmd line character into integer
  int rounds = atoi(argv[1]);

  int pid1, pid2;
  int child1_win = 0;
  int child2_win = 0;
  int signal; //1 if need to throw or 0 if the game is over
  int firstThrow, secondThrow;
  int i, readMessage, choice, status;
  char readPipe[100];
  char chose[3];

  //set these up for random numbers
  time_t t;
  srand(time(0));



  if ( pid1 = fork() ) { //first child process

    struct sockaddr_un sa;
    int socketfd, check_connection;

    //create the socket for the first child
    printf("...Child Process is about to create socket...\n");
    socketfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketfd < 0) {
      perror("...Error creating scket for fist child process.....");
      exit(-1);
    }
    sa.sun_family = AF_UNIX;
    strcpy(sa.sun_path, SOCKET_PATH);
    int length = sizeof(sa);

    //connect to the server
    printf("...child 1 socket connecting to server......\n");
    for (i = 0; i < 10; i++) {
      check_connection = connect(socketfd, (struct sockaddr * ) &sa, length);
      if (check_connection < 0) {
        usleep(10000);
        perror("....Connection failed on child 1... Im going to keep trying......");
      } else {  break; }
    }
    if (check_connection == -1) {
      perror("Connection Error! Unable to connect!");
      exit(-1);
    }
    printf("Child 1 Socket Connection Successful!!!\n");

    //write to the parent process
    write(socketfd, "Child 1 Ready", strlen("Child 1 Ready")+1 );

    for (i = 0; i < rounds; i++) {
      //read from the parent processe
      readMessage = read(socketfd, readPipe, 100);

      //choose the choice
      srand(time(0));
      choice = (rand() % 3) + 1;

      //change into a string
      sprintf(chose, "%d", choice);

      //write the choice back to the parent through the pipe
      write(socketfd, chose, strlen(chose) + 1);

    }

    //close the socket
    close(socketfd);
  }

  else if ( pid2 = fork() ) { //second child process

    //create the socket for the second child
    struct sockaddr_un sa2;
    int socketfd2, check_connection2;
    int length2;
    printf("...Child Process 2 is about to create socket...\n");
    socketfd2 = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketfd2 < 0) {
      perror("...Error creating scket for second child process.....");
      exit(-1);
    }
    sa2.sun_family = AF_UNIX;
    strcpy(sa2.sun_path, SOCKET_PATH);
    length2 = sizeof(sa2);

    //connect to the server
    printf("...child 2 socket connecting to server......\n");
    for (i = 0; i < 10; i++) {
      check_connection2 = connect(socketfd2, (struct sockaddr * ) &sa2, length2);
      if (check_connection2 < 0) {
        usleep(10000);
        perror("....Connection failed on child 2 ... Im going to keep trying......");
      } else {  break; }
    }

    //write to the parent process
    write(socketfd2, "Child 2 Ready", strlen("Child 2 Ready")+1 );

    for (i = 0; i < rounds; i++) {
       readMessage = read( socketfd2, readPipe, 100); //read from the parent process
       sleep(1);
       //choose the choice
       srand(time(0));
       choice = (rand() % 3) + 1;

       //change into a string
       sprintf(chose, "%d", choice);

       //write the choice back to the parent through the pipe
       write(socketfd2, chose, strlen(chose) + 1);
    }

    //close the socket
    close(socketfd2);

  }

  else { //parent process

    struct sockaddr_un serv_addr;
    int socketfd_parent, c1, c2;

    //create the socket for the parent process
    socketfd_parent = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketfd_parent < 0) {
      perror("...Error creating the socket fr the parent process!....");
      exit(-1);
    }
    //setup the socket address
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, SOCKET_PATH);

    //clear the socket
    unlink(SOCKET_PATH);

    //bind to socket
    printf("....Attempting to bind.....\n");
    int size = sizeof(serv_addr);
    if(bind(socketfd_parent, (struct sockaddr *) &serv_addr, size) < 0 ) {
            perror("....Error attempting to bind!.....r\n");
            exit(-1);
    }

    //listening to the socket
    printf("...Attempting to listen to the socket...\n");
    if(listen(socketfd_parent, 1) < 0) {
            perror("listen error\n");
            exit(-1);
    }

    //accepting first child
    printf("... Attempting to accept first child...\n");
    c1 = accept(socketfd_parent, NULL, NULL);
    if( c1 < -1) {
            perror("Error accepting first child!....\n");
            exit(-1);
    }

    //accepting second child
    printf("... Attempting to accept second child...\n");
    c2 = accept(socketfd_parent, NULL, NULL);
    if( c2 < -1) {
            perror("Error accepting second child!....\n");
            exit(-1);
    }


    readMessage = read(c1, readPipe, 100);
    printf(readPipe);
    printf("\n");
    readMessage = read(c2, readPipe, 100);
    printf(readPipe);
    printf("\n");
    printf("Beginning %d Rounds.....\n", rounds);
    printf("Fight!\n");
    printf("-----------------------------------------\n");

    for (i = 0; i < rounds; i++) {
      printf("Round %d: \n",i );
      //signal to the two processes
      write(c1, "Go", strlen("Go") + 1);
      write(c2, "Go", strlen("Go") + 1);

      //read pipes from the child1
      readMessage = read( c1, readPipe, 100);
      if ( strcmp(readPipe,"1" ) == 0 ) { printf( "Child 1 throws Rock!\n" ); }
      if ( strcmp(readPipe,"2" ) == 0 ) { printf( "Child 1 throws Paper!\n" ); }
      if ( strcmp(readPipe,"3" ) == 0 ) { printf( "Child 1 throws Scissors!\n" ); }

      /*** child2 convert it into a number ***/
      firstThrow = atoi(readPipe);

      //read pipes from child2
      readMessage = read( c2, readPipe, 100);
      if ( strcmp(readPipe,"1" ) == 0 ) { printf( "Child 2 throws Rock!\n" ); }
      if ( strcmp(readPipe,"2" ) == 0 ) { printf( "Child 2 throws Paper!\n" ); }
      if ( strcmp(readPipe,"3" ) == 0 ) { printf( "Child 2 throws Scissors!\n" ); }

      /*** child1 convert it into a number**/
      secondThrow = atoi(readPipe);

      if ( firstThrow == 1 &&  secondThrow ==2 ) {printf("Child 2 Won!\n"); child2_win++; }
      if ( firstThrow == 1 &&  secondThrow ==3 ) {printf("Child 1 Won!\n"); child1_win++; }
      if ( firstThrow == 2 &&  secondThrow ==1 ) {printf("Child 2 Won!\n"); child2_win++; }
      if ( firstThrow == 2 &&  secondThrow ==3 ) {printf("Child 2 Won!\n"); child2_win++; }
      if ( firstThrow == 3 &&  secondThrow ==1 ) {printf("Child 1 Won!\n"); child1_win++; }
      if ( firstThrow == 3 &&  secondThrow ==2 ) {printf("Child 1 Won!\n"); child1_win++; }
      if ( firstThrow == secondThrow) {printf("Tie!!!!...So Noboody Wins or Looses!\n"); }
      printf("-----------------------------------------\n");

    }

    wait(NULL);
    close(socketfd_parent);

  }
  return 0;
}
