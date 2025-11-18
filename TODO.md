### buf_rcvd_t
- заменить типы buf_rcvd_t, buf_xmit_t на единый serial_buf_t

### msg_pack
- в msg_pack при создании crc один и тоже макрос вызывается для разных CRC_TYPE

### transp
данные, которые нужно прокинуть через data_if_t:
- static char port = 0x01;
- topmode_t _mode = mode 

### S_TO_PB
- S_TO_PB(buf+*size, self->answ_buf[i]) здесь answ_buf это u16_t, значит терям
  минуса. надо заменить