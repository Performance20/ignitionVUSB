

#include <stdbool.h>
#include "ringbf.h" 

static ring_buffer_t tx_buffer = { { 0 }, 0, 0 };

int store_char(uchar c, ring_buffer_t *the_buffer, uchar ow)
{
  int i = (the_buffer->head + 1) % RING_BUFFER_SIZE;
  if (ow == 0)
  {
	  if (i != the_buffer->tail) {
		  the_buffer->buffer[the_buffer->head] = c;
		  the_buffer->head = i;
		  return 1;
	  }
	  return 0;
  }
  else
  {
	  the_buffer->buffer[the_buffer->head] = c;
	  the_buffer->head = i;
	  if (i == the_buffer->tail)
		  the_buffer->tail = (the_buffer->tail + 1) % RING_BUFFER_SIZE;
	  return 1;
  }
}

int tx_write(uchar c, uchar overwrite) { // stop writing, when buffer is full and overwrite is false
	//if (!tx_remaining()) tx_read();
    return store_char(c, &tx_buffer, overwrite);
}

int tx_writeow(uchar c) { // overwrite if buffer is full
	return store_char(c, &tx_buffer, true);
}


int tx_remaining() {
  int t;
  t = RING_BUFFER_SIZE - (RING_BUFFER_SIZE + tx_buffer.head - tx_buffer.tail) % RING_BUFFER_SIZE;
  return t;
}
  
int tx_available() {
	int t;
    t = (RING_BUFFER_SIZE + tx_buffer.head - tx_buffer.tail) % RING_BUFFER_SIZE;
	return t;
}

uchar tx_read() {
	uchar t;
  // if the head isn't ahead of the tail, we don't have any characters to send
  if (tx_buffer.head == tx_buffer.tail) {
    t =  0;
  } else {
    uchar c = tx_buffer.buffer[tx_buffer.tail];
    tx_buffer.tail = (tx_buffer.tail + 1) % RING_BUFFER_SIZE;
    t =  c;
  } 
  return t;
}

#ifdef RING_BUFFER_RX
static ring_buffer_t rx_buffer = { { 0 }, 0, 0 };

int rx_available() {
	int t;
	t =  (RING_BUFFER_SIZE + rx_buffer.head - rx_buffer.tail) % RING_BUFFER_SIZE;
	return t;
}


int rx_read() {
	int t;
	// if the head isn't ahead of the tail, we don't have any characters
	if (rx_buffer.head == rx_buffer.tail) {
		t = -1;
		} else {
		unsigned char c = rx_buffer.buffer[rx_buffer.tail];
		rx_buffer.tail = (rx_buffer.tail + 1) % RING_BUFFER_SIZE;
		t = c;
	}
	return t;
}
#endif
