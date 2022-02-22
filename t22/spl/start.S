	.globl start
	.section ".text"
_start:
	b	reset_Handler
	bl	Undefined_Handler
	bl	SVC_Handler
	bl	Prefetch_Handler
	bl	Abort_Handler
	bl	Hypervisor_Handler
	b	IRQHandler
	bl	FIQ_Handler

	.extern enableVGIC
	.extern 


























