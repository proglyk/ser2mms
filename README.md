### ser2mms - serial to MMS (Manufacturing Message Specification) conversion utility

#### State
Is not stable and still under development...

#### Base configuration (HAL depends of OS)

```sh
make                        // default Linux x86_64
make ARCH=x86_64 OS=linux   // or specific
make ARCH=x86_64 OS=win     // Windows (cross compile)
make ARCH=arm    OS=linux   // Linux ARM
make ARCH=arm    OS=rtos    // ARM runned under RTOS
make ARCH=arm    OS=bare    // Bare-metal ARM
```

#### How to use
```c
s2m_t *s2m 
s2m = ser2mms_new(
  NULL,       // MMS stack
  write_carg, // Functor
  write_subs, // Functor
  read_answ,  // Functor
  S2M_SLAVE,  // Mode (SLAVE or POLL)
  12,         // Address 
  &stty_init  // Config for ssty
);
                         
ser2mms_run(s2m);

while (runned) {
  // work
}

ser2mms_stop(s2m);
```