#include "shell.h"
#include <stddef.h>
#include "clib.h"
#include <string.h>
#include "fio.h"
#include "filesystem.h"
//#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "host.h"

typedef struct {
	const char *name;
	cmdfunc *fptr;
	const char *desc;
} cmdlist;

void ls_command(int, char **);
void man_command(int, char **);
void cat_command(int, char **);
void ps_command(int, char **);
void host_command(int, char **);
void help_command(int, char **);
void host_command(int, char **);
void mmtest_command(int, char **);
void new_command(int, char **);
void test_command(int, char **);
void _command(int, char **);

#define MKCL(n, d) {.name=#n, .fptr=n ## _command, .desc=d}

cmdlist cl[]={
	MKCL(ls, "List directory"),
	MKCL(man, "Show the manual of the command"),
	MKCL(cat, "Concatenate files and print on the stdout"),
	MKCL(ps, "Report a snapshot of the current processes"),
	MKCL(host, "Run command on host"),
	MKCL(mmtest, "heap memory allocation test"),
	MKCL(help, "help"),
	MKCL(test, "find fibonacci num"),
	MKCL(new, "create a new task"),
	MKCL(, ""),
};

int parse_command(char *str, char *argv[]){
	int b_quote=0, b_dbquote=0;
	int i;
	int count=0, p=0;
	for(i=0; str[i]; ++i){
		if(str[i]=='\'')
			++b_quote;
		if(str[i]=='"')
			++b_dbquote;
		if(str[i]==' '&&b_quote%2==0&&b_dbquote%2==0){
			str[i]='\0';
			argv[count++]=&str[p];
			p=i+1;
		}
	}
	/* last one */
	argv[count++]=&str[p];

	return count;
}

void ls_command(int n, char *argv[]){
    fio_printf(1,"\r\n"); 
    int dir;
    if(n == 0){
        dir = fs_opendir("");
    }else if(n == 1){
        dir = fs_opendir(argv[1]);
        //if(dir == )
    }else{
        fio_printf(1, "Too many argument!\r\n");
        return;
    }
(void)dir;   // Use dir
}

int filedump(const char *filename){
	char buf[128];

	int fd=fs_open(filename, 0, O_RDONLY);

	if( fd == -2 || fd == -1)
		return fd;

	fio_printf(1, "\r\n");

	int count;
	while((count=fio_read(fd, buf, sizeof(buf)))>0){
		fio_write(1, buf, count);
    }
	
    fio_printf(1, "\r");

	fio_close(fd);
	return 1;
}

void ps_command(int n, char *argv[]){
	signed char buf[1024];
	vTaskList(buf);
        fio_printf(1, "\n\rName          State   Priority  Stack  Num\n\r");
        fio_printf(1, "*******************************************\n\r");
	fio_printf(1, "%s\r\n", buf + 2);	
}

void cat_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: cat <filename>\r\n");
		return;
	}

    int dump_status = filedump(argv[1]);
	if(dump_status == -1){
		fio_printf(2, "\r\n%s : no such file or directory.\r\n", argv[1]);
    }else if(dump_status == -2){
		fio_printf(2, "\r\nFile system not registered.\r\n", argv[1]);
    }
}

void man_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: man <command>\r\n");
		return;
	}

	char buf[128]="/romfs/manual/";
	strcat(buf, argv[1]);

    int dump_status = filedump(buf);
	if(dump_status < 0)
		fio_printf(2, "\r\nManual not available.\r\n");
}

void host_command(int n, char *argv[]){
    int i, len = 0, rnt;
    char command[128] = {0};

    if(n>1){
        for(i = 1; i < n; i++) {
            memcpy(&command[len], argv[i], strlen(argv[i]));
            len += (strlen(argv[i]) + 1);
            command[len - 1] = ' ';
        }
        command[len - 1] = '\0';
        rnt=host_action(SYS_SYSTEM, command);
        fio_printf(1, "\r\nfinish with exit code %d.\r\n", rnt);
    } 
    else {
        fio_printf(2, "\r\nUsage: host 'command'\r\n");
    }
}

void help_command(int n,char *argv[]){
	int i;
	fio_printf(1, "\r\n");
	for(i = 0;i < sizeof(cl)/sizeof(cl[0]) - 1; ++i){
		fio_printf(1, "%s - %s\r\n", cl[i].name, cl[i].desc);
	}
}

void task(void *pvParameters)
{
	fio_printf(1,"I'm NEW task\r\n");	
	while(1);
}

void new_command(int n, char *argv[])
{
	fio_printf(1,"\r\n");
	xTaskCreate(task,(signed portCHAR *) "I'm NEW",
		    512,NULL,tskIDLE_PRIORITY + 1,NULL);

	int handle;
	int error1;
	int error2;
	int error3;
	handle = host_action(SYS_SYSTEM, "mkdir -p System-info");
	handle = host_action(SYS_SYSTEM, "touch System-info/sysinfo");
	handle = host_action(SYS_OPEN, "System-info/sysinfo", 8);
	if(handle == -1) {
		fio_printf(1, "Open file error!\n\r");
		return;
	}

	signed char buf[1024];
	vTaskList(buf);
        //fio_printf(1, "\n\rName          State   Priority  Stack  Num\n\r");
        //fio_printf(1, "*******************************************\n\r");
	//fio_printf(1, "%s\r\n", buf + 2);
	

	//char *buffer = "Test host_write function which can write data to output/syslog\n";
	char *buffer1 = "\r\nName          State   Priority  Stack  Num\n\r";
	char *buffer2 = "*******************************************\n\r";
	signed char *buffer3 = (buf + 2);
	error1 = host_action(SYS_WRITE, handle, (void *)buffer1, strlen(buffer1));
	error2 = host_action(SYS_WRITE, handle, (void *)buffer2, strlen(buffer2));
	error3 = host_action(SYS_WRITE, handle, (void *)buffer3, strlen((const char*)buffer3));
	
	if(error1 != 0 || error1 != 0 || error1 != 0)
	{
		if(error1 != 0)
			fio_printf(1, "Write file error! Remain %d bytes didn't write in the file.\n\r", error1);

		if(error2 != 0 )
			fio_printf(1, "Write file error! Remain %d bytes didn't write in the file.\n\r", error2);

		if(error3 != 0)
			fio_printf(1, "Write file error! Remain %d bytes didn't write in the file.\n\r", error3);

		host_action(SYS_CLOSE, handle);
		return;
	}

	host_action(SYS_CLOSE, handle);
}

void test_command(int n, char *argv[]) {

	int num=0;	
	if(n==2)
	{
		int length = strlen(argv[1]);
		fio_printf(1,"\r\n");

		for(int i = 0; i < length; i++)
			num = num * 10 + (argv[1][i]-'0');

		int previous = -1;
		int result = 1;
		int sum = 0;

		for(int i = 0; i <= num; i++)
		{
			sum = result + previous;
			previous = result;
			result = sum;		
		}

		fio_printf(1,"Fibonacci : number %d is %d \r\n\r\n",num, result);
	}
	else
		fio_printf(1,"\r\ninput parameter need to be less or equal to 2\r\n\r\n");
}

void _command(int n, char *argv[]){
    (void)n; (void)argv;
    fio_printf(1, "n");
}

cmdfunc *do_command(const char *cmd){

	int i;

	for(i=0; i<sizeof(cl)/sizeof(cl[0]); ++i){
		if(strcmp(cl[i].name, cmd)==0)
			return cl[i].fptr;
	}
	return NULL;	
}
