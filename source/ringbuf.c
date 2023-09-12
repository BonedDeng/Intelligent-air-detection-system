#include "main.h"

#ifdef USE_RINGBUF

/*---------------------------------------------------------------------------*/
/**
 * \brief      Initialize a ring buffer
 * \param r    A pointer to a struct ringbuf to hold the state of the ring buffer
 * \param a    A pointer to an array to hold the data in the buffer
 * \param size_power_of_two The size of the ring buffer, which must be a power of two
 *
 *             This function initiates a ring buffer. The data in the
 *             buffer is stored in an external array, to which a
 *             pointer must be supplied. The size of the ring buffer
 *             must be a power of two and cannot be larger than 128
 *             bytes.
 *
 */
void
ringbuf_init(struct ringbuf *r, uint8_t *dataptr, uint8_t size)
{
  r->dat = dataptr;
  r->mask = size - 1;
  r->put_ptr = 0;
  r->get_ptr = 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Insert a byte into the ring buffer
 * \param r    A pointer to a struct ringbuf to hold the state of the ring buffer
 * \param c    The byte to be written to the buffer
 * \return     Non-zero if there data could be written, or zero if the buffer was full.
 *
 *             This function inserts a byte into the ring buffer. It
 *             is safe to call this function from an interrupt
 *             handler.
 *
 */
int
ringbuf_put(struct ringbuf *r, uint8_t c)
{
  /* Check if buffer is full. If it is full, return 0 to indicate that
     the element was not inserted into the buffer.

     XXX: there is a potential risk for a race condition here, because
     the ->get_ptr field may be written concurrently by the
     ringbuf_get() function. To avoid this, access to ->get_ptr must
     be atomic. We use an uint8_t type, which makes access atomic on
     most platforms, but C does not guarantee this.
  */
  if(((r->put_ptr - r->get_ptr) & r->mask) == r->mask) {
    return 0;
  }
  r->dat[r->put_ptr] = c;
  r->put_ptr = (r->put_ptr + 1) & r->mask;
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Get a byte from the ring buffer
 * \param r    A pointer to a struct ringbuf to hold the state of the ring buffer
 * \return     The data from the buffer, or -1 if the buffer was empty
 *
 *             This function removes a byte from the ring buffer. It
 *             is safe to call this function from an interrupt
 *             handler.
 *
 */
int
ringbuf_get(struct ringbuf *r)
{
  uint8_t c;
  
  /* Check if there are bytes in the buffer. If so, we return the
     first one and increase the pointer. If there are no bytes left, we
     return -1.

     XXX: there is a potential risk for a race condition here, because
     the ->put_ptr field may be written concurrently by the
     ringbuf_put() function. To avoid this, access to ->get_ptr must
     be atomic. We use an uint8_t type, which makes access atomic on
     most platforms, but C does not guarantee this.
  */
  if(((r->put_ptr - r->get_ptr) & r->mask) > 0) {
    c = r->dat[r->get_ptr];
    r->get_ptr = (r->get_ptr + 1) & r->mask;
    return c;
  } else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Get the size of a ring buffer
 * \param r    A pointer to a struct ringbuf to hold the state of the ring buffer
 * \return     The size of the buffer.
 */
int
ringbuf_size(struct ringbuf *r)
{
  return r->mask + 1;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Get the number of elements currently in the ring buffer
 * \param r    A pointer to a struct ringbuf to hold the state of the ring buffer
 * \return     The number of elements in the buffer.
 */
int
ringbuf_elements(struct ringbuf *r)
{
  return (r->put_ptr - r->get_ptr) & r->mask;
}
/*---------------------------------------------------------------------------*/
#endif//USE_RINGBUF

