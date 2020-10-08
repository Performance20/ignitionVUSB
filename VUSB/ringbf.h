
#ifndef __ringbf_h_included__
#define __ringbf_h_included__


#define RING_BUFFER_SIZE 50

//#undef RING_BUFFER_RX

#ifndef uchar
#define uchar   unsigned char
#endif

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct {
		uchar buffer[RING_BUFFER_SIZE];
		int head;
		int tail;
	} ring_buffer_t;

	int store_char(unsigned char c, ring_buffer_t* the_buffer, uchar overwrite);


	int tx_remaining(void);
	int tx_write(uchar c, uchar overwrite);
	int tx_writeow(uchar c); //overwrite version
	int tx_available();
	uchar tx_read();

#ifdef RING_BUFFER_RX
	int rx_available();
	int rx_read();
#endif
#ifdef __cplusplus
}
#endif

#endif  // __ringbf_h_included__


