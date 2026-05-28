
## pid

```c
void pi_init(PI_t* analyzer, float kp, float ki, float outmin, float outmax);
float pi_update(PI_t* analyzer, float now, float target);
```