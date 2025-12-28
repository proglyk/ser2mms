### ser2mms - serial to MMS (Manufacturing Message Specification) conversion utility

#### State
Is not stable and still under development...

#### Base configuration (HAL depends of OS)

```sh
make                        // default
make ARCH=x86_64 OS=linux   // WSL
make ARCH=arm    OS=linux   // Linux ARM
make ARCH=arm    OS=rtos    // ARM runned under RTOS
```

#### How to use
```c
// Ied model
extern IedModel iedModel;

// stty configuration
static rs485_init_t rs485_init = {
  .device_path = "/dev/ttyS2",
  .gpio_path   = "/dev/gpiochip1",
  .gpio_pin    = P9_23
};

int
main(int argc, char** argv)
{
  // Create and run IedServer
  IedServer iedServer = IedServer_create(&iedModel);
  IedServer_start(iedServer, 102);
  if (!IedServer_isRunning(iedServer)) { /* ... */ }

  // Create and run s2m in 'slave' mode
  s2m_t *s2m = ser2mms_new( (void *)iedServer, 
    S2M_SLAVE, 12, (void *)&rs485_init );
  if (!s2m) { /* ... */ }
  ser2mms_run(s2m);

  // ...

  // Clean resources
  ser2mms_destroy(s2m);
  IedServer_stop(iedServer);
  IedServer_destroy(iedServer);
}
```
#### Reading page values
```c
 void ser2mms_read_page(const page_prm_t *buf, u8_t ds, u8_t page, void *opaque)
{
  assert(opaque);
  if ((void *ied = ser2mms_get_ied((s2m_t *)opaque)) == NULL) { return; }

  switch (ds)
  {    
    case (1): {
      switch (page)
      {
        case (0): S2M_SET_PARAMS_F32(LogicalNode1, P1, P2, P3, buf); break;
        case (1): S2M_SET_PARAMS_S32(LogicalNode1, P4, P5, P6, buf); break;
        // ...
        case (Q): S2M_SET_PARAMS_S32(LogicalNode1, P(X-2), P(X-1), PX, buf); break;
      }
    } break;

    // ...

    case (N): {
      switch (page)
      {
        case (0): S2M_SET_PARAMS_F32(LogicalNodeN, P1, P2, P3, buf); break;
        case (1): S2M_SET_PARAMS_S32(LogicalNodeN, P4, P5, P6, buf); break;
        // ...
        case (Q): S2M_SET_PARAMS_S32(LogicalNodeN, P(X-2), P(X-1), PX, buf); break;
      }
    } break;
  }
}
```
// where Q - page size (SER_MAX_PAGE_IDX),
// N - number of datasets (SER_MAX_DS_IDX)

#### Reading subscription values
```c

void ser2mms_read_subs(const sub_prm_t *buf, void *opaque)
{
  assert(opaque);
  if ((void *ied = ser2mms_get_ied((s2m_t *)opaque)) == NULL) { return; }

  S2M_SET_ATTRS_S32(LogicalNode1, Attribute1, buf[0].mag, buf[0].t, true);
  S2M_SET_ATTRS_S32(LogicalNode1, Attribute2, buf[1].mag, buf[1].t, true);
  // ...
  S2M_SET_ATTRS_S32(LogicalNode1, AttributeN, buf[M].mag, buf[N].t, true);
}
```
// where M - subscription array size (SER_NUM_SUBS)

#### Writing answer
```c
 void ser2mms_write_answer(answ_prm_t *buf, u32_t *buf_len)
{
  u32_t cnt = 0;

  DataAttribute *data_attr = (DataAttribute*)IEDMODEL_UPG_GGIO0_Circ_mag_i;
  buf[cnt++].mag = (s16_t)( MmsValue_toInt32(data_attr->mmsValue) & 0x0000ffff );

  *buf_len = cnt;
}
```

