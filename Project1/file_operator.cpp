#include "file_operator.h"
#include "base_log.h"
#include <linux/errno.h>
#include <string.h>


file_accessor::file_accessor(const char* file_name) : file_name(file_name) {
	LOGD("file_accessor init , file_name :  %s", file_name);
}

file_accessor::~file_accessor(void) {
	LOGD("enter  ~file_accessor");
 	fclose(read_stream);
	fclose(write_stream);
	LOGI("errno msg : %s", strerror(errno));
}

void *
file_accessor::read_file()  {

	char * read_result = NULL;
	
	if (0 == access(this->file_name, F_OK)){
		 read_stream = fopen(this->file_name, "r+t");
 
		 LOGE("fopen result : %s", strerror(errno));

		 fseek(read_stream, 0, SEEK_END);
		 long lsize = ftell(read_stream);
		 rewind(read_stream);
		 LOGD("lsize = %d", lsize);
		 read_result = (char *)malloc(lsize * sizeof(char));

		 memset(read_result, 0, 19 * sizeof(char));
		 fread(read_result, sizeof(char), 19, read_stream);
		 LOGD("read from file result : %s", strerror(errno));
	}
	else {
		read_result = "The file is not exits.";
	}

	LOGD("read_result : %s", read_result);
	fclose(read_stream);

	return (void*)read_result;
}

int 
file_accessor::put_file(void* args, size_t len){

	write_stream = fopen(this->file_name, "wt");
	int res = fwrite(args, 1, len, write_stream);
	LOGI("res = %d", res);
	LOGE("fwrite  result  message :  %s",strerror(errno));
	fclose(write_stream);

	return res;
}

