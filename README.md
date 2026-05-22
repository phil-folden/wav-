# wav-
第一版：TIM3更新事件触发DMA更改TIM2->CCR1，从而改变占空比。
但这一版存在问题：如果想要两个声道，则需要同时改变TIM2->CCR1和TIM2->CCR2。
这对普通DMA难以实现，因为DMA传输给的地址是固定的。
所以下一版的双声道会把TIM3定时器转到TIM1高级定时器上，同时用TIMER_DMA_BURST同时改变CCR1和CCR2
