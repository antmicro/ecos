#ifndef __MCC_API__
#define __MCC_API__

int mcc_initialize(unsigned int);
int mcc_destroy(unsigned int);
int mcc_create_endpoint(unsigned int*, unsigned int);
int mcc_destroy_endpoint(unsigned int*);
int mcc_send(unsigned int*, void*, unsigned int, unsigned int);
int mcc_recv_copy(unsigned int*, void*, unsigned int, unsigned int*, unsigned int);
int mcc_recv_nocopy(unsigned int*, void**, unsigned int*, unsigned int);
int mcc_msgs_available(unsigned int*, unsigned int*);
int mcc_free_buffer(void*);
int mcc_get_info(unsigned int, char*);

#endif /* __MCC_API__ */
