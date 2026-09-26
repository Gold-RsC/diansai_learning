
## pid

```c
void PI_Init(PI_t* analyzer, float kp, float ki, float outmin, float outmax);
float PI_Update(PI_t* analyzer, float now, float target);
```