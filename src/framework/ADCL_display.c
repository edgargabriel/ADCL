#include <ADCL_internal.h>
#define MAGIC 918273
#define TCP_BUFFER_SIZE  262142
#define MAXLEN 20
#define MAXMSGBUFSIZE 1024

char ADCL_display_ip[MAXLEN] = "172.25.66.95";
int ADCL_display_port = 20000;
int ADCL_display_rank = 0;
int ADCL_display_flag = 0;
static char strres[128];

static int sock;
char *hostname, *msgbuf;
int selfrank =0;

/* Prototypes */

typedef struct
{
int magic;//magic number to verify the accuracy of the transmission.
int type;//type of message. either coordinates or message to be displayed.
int len;//length of the message.
int id;//id is to specify which tab to update.
double yvalue;
}header;

static void configure_socket ( int sd );
static void write_string ( int hdl, char *buf, int num );
static void read_string ( int hdl, char *buf, int num );
static void write_int ( int hdl, int val );
static void read_int ( int hdl, int *val );
static void write_header(int hdl, header head);
static void endian_init (void );

int ADCL_display_init()
{
	char* ip = ADCL_display_ip;
	int portnum = ADCL_display_port;
	int rank = ADCL_display_rank;
    	int port, msglen;
    	struct sockaddr_in client;
    	struct hostent *host;
	 MPI_Comm_rank ( MPI_COMM_WORLD, &selfrank );
	if(ADCL_display_flag && rank == selfrank)
	{
	 hostname = strdup ( ip );
   	 port     = portnum;

    	/* convert the hostname to an IP address */
   	if ( ( host = gethostbyname ( hostname ) ) == NULL ) {
        	printf("CLIENT: could not resolve hostname %s \n", hostname );
        	return -1;
    	}


    	if ( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        	printf("CLIENT: could not get a socket\n" );
    	}
    	else {
        	printf("CLIENT: got a a socket\n");
    	}

	client.sin_family      = AF_INET;
    	client.sin_port        = htons(port);
   	memcpy ( &client.sin_addr, host->h_addr, host->h_length );
    	configure_socket(sock);
    	if ( connect (sock, (struct sockaddr *)&client, sizeof(client)) < 0) {
       	 	printf("CLIENT: connect call failed to %s on port %d \n", hostname, port);
    	}
    	else {
        	printf("CLIENT: connection established to %s on port %d\n", hostname, port);
    	}
		
    		write_int ( sock, sizeof(header) );
	}
    	return 0;
}

int ADCL_display(int type,...)
{
	if(ADCL_display_flag && ADCL_display_rank == selfrank)
	{
		header head = {MAGIC, 0, 0, 0,0.0};

	head.type = type;
	va_list ap;
	va_start(ap,type);
	if(ADCL_DISPLAY_POINTS == type)
	{
		head.id = va_arg(ap,int);
		head.yvalue = va_arg(ap,double);
		write_header(sock,head);
	}
	else if(ADCL_DISPLAY_MESSAGE == type)
	{
		char msg[MAXMSGBUFSIZE];
		head.id = va_arg(ap, int);
		vsprintf(msg,va_arg(ap, char*),ap);
		head.len = strlen(msg);
		write_header(sock,head);
		write_string(sock,msg,head.len);
	}
	va_end(ap);
	}
	return ADCL_SUCCESS;
}

int ADCL_display_finalize()
{
	if(ADCL_display_flag && selfrank == ADCL_display_rank)
	{
		header head = {MAGIC,0,0,0,0.0};
		head.type = ADCL_DISPLAY_COMM_FINAL;
		write_header(sock,head);
    		//needs to be taken care of.
		char*  msgbuf = (char *) malloc ( 32 );
    		read_string ( sock, msgbuf, 32);

		/* After everything is done, close the socket */
    		close (sock);
    		printf("CLIENT: connection closed \n");
    		/* A string allocated with strdup has to be freed again */
    		free ( hostname );
    		free ( msgbuf );
	}
}

void configure_socket ( int sd )
{

    int flag;
    struct linger ling;

    flag=1;
    if(setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag))<0) {
        printf("configure_socket: could not configure socket\n");
    }

    flag=TCP_BUFFER_SIZE;
    if(setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (char*)&flag, sizeof(flag))<0) {
        printf("configure_socket: could not configure socket\n");
    }

    flag=TCP_BUFFER_SIZE;
    if(setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (char*)&flag, sizeof(flag))<0) {
        printf("configure_socket: could not configure socket\n");
    }

    flag=1;
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag))<0) {
        printf("configure_socket: Could not set SO_REUSEADDR\n");
    }
                                          
    ling.l_onoff  = 1;
    ling.l_linger = 0;
    if(setsockopt(sd, SOL_SOCKET, SO_LINGER, &ling, sizeof(struct linger))<0) {
        printf("configure_socket: Could not set SO_LINGER\n");
    }

    return;
}
static int host_is_little = 0;
static int is_init=0;

void write_string ( int hdl, char *buf, int num )
{
  int a;
  char *wbuf = buf;

  if ( !is_init ) {
      endian_init ();
  }
  do {
    a = write ( hdl, wbuf, num);
    if ( a == -1 ) {
        if ( errno == EINTR ) {
            continue;
        }
        else
        {
            strerror ( errno );
            printf ("write_string: error while writing to socket");
            exit(-1);
        }
    }
    wbuf += a;
    num -= a;
  } while ( num > 0 );
}

void read_string ( int hdl, char *buf, int num )
{
  int a;
  char *wbuf = buf;

  if ( !is_init ) {
      endian_init ();
  }


  do {
 a = read ( hdl, wbuf, num);
      if ( a == -1 ) {
          if ( errno == EINTR ) {
              continue;
          }
          else
          {
              strerror ( errno );
              printf("read_string: error while reading from socket");
              exit(-1);
          }
      }
    num -= a;
    wbuf += a;
  } while ( num > 0 );
}

void write_int ( int hdl, int val )
{
  char o_p[4];
  char *buf;
  char *wbuf;
  long lval;
  short sval;

  if ( !is_init ) {
      endian_init ();
  }

  /* Make sure, you have a 4 byte integer */
  if ( sizeof(int) == 4) {
    buf = (char *) &val;
  }
  else if ((sizeof(int) == 8) && (sizeof(short) == 4)) {
      sval = (short) val;
      buf  = (char*) &sval;
  }
  else if ((sizeof(int) == 2) && (sizeof(long)  == 4)) {
      lval = (long)  val;
      buf  = (char*) &lval;
  }
else {
    printf ("write_int: Unknown data format for integer");
    exit (-1);
  }

  /* Make sure you send big-endian */
  if ( host_is_little ) {
      o_p[0] = buf[3];
      o_p[1] = buf[2];
      o_p[2] = buf[1];
      o_p[3] = buf[0];
      wbuf = o_p;
    }
  else {
    wbuf = buf;
  }

  /* Write the data */
  write_string ( hdl, wbuf, 4 );
}


void read_int ( int hdl, int *val )
{
  char o_p[4], i_p[4];
  char *wbuf = o_p;
  char *buf;
  int   *ival;
  short *sval;
  long  *lval;

  if ( !is_init ) {
      endian_init ();
  }


  /* Read the data */
  read_string ( hdl, wbuf, 4 );

  /* Make sure, data has host representation */
  if ( host_is_little )  {
      i_p[0] = o_p[3];
      i_p[1] = o_p[2];
      i_p[2] = o_p[1];
      i_p[3] = o_p[0];
      buf = i_p;
  }
  else {
    buf = o_p;
  }

  /* Make sure, that the 4 byte integer is converted into a local int */
  if ( sizeof(int) == 4) {
      ival = (int*) buf;
      *val  = *ival;
  }
  else if ( (sizeof(int) == 8) && (sizeof(short) == 4)) {
      sval = (short*) buf;
      *val = (int) *sval;
  }
  else if ( (sizeof(int) == 2) && (sizeof(long) == 4 )) {
      lval = (long*) buf;
      *val = (int) *lval;
  }
  else {
    printf("read_int: unkwon data format for integer");
    exit (-1);
  }
}


void write_header ( int hdl, header head )
{
   char *msg = (char*)&head;
   write_string(hdl,msg,sizeof(head));
}

static void endian_init ()
{
  int x=1;
  host_is_little = (*(char*)&x==1);

  is_init = 1;
}
             
