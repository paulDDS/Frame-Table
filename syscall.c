#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);

//Validate a single adress
static void validate_ptr(const void *ptr) {
  if (ptr == NULL ||
      !is_user_vaddr(ptr) ||
      pagedir_get_page(thread_current()->pagedir, ptr) == NULL)
  {
    printf("%s: exit(%d)\n", thread_name(), -1);
    thread_exit();
  }
}

//Validate each byte in a range
static void validate_stack(const void *addr, size_t size)
{
  for (size_t i = 0; i < size; i++)
    validate_ptr((const uint8_t *)addr + i);
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // printf("HEXDUMP ESP\n");

  // uint32_t *hesp = (uint32_t *) f->esp;

  // for (int i = 0; i < 8; i++) {
  //   printf("esp[%d] = 0x%x\n", i, hesp[i]);
  // }

  //Valida
  validate_ptr(f->esp);
  int *esp = (int *) f->esp;
  int syscall = esp[0];

  switch(syscall)
  {
    case SYS_WRITE:
    {
      validate_stack(esp, 12);

      int fd = esp[1];
      const char *buffer = (const char *) esp[2];
      unsigned size = esp[3];

      validate_stack(buffer, 0);
      validate_stack((const uint8_t *)buffer, size - 1);

      if (fd == 1)
      {
        putbuf(buffer, size);
        f->eax = size;
      }
      else
      {
        f->eax = -1;
      }
      break;
    }
    case SYS_WAIT:
    {
      validate_stack(esp, 4);

      int pid = esp[1];
      f->eax = process_wait(pid);
      break;
    }
    case SYS_EXIT:
    {
      validate_stack(esp, 4);

      int exit_code = esp[1];
      printf ("%s: exit(%d)\n", thread_name(), exit_code);
      thread_exit ();
      break;
    }
    default:
    {
      printf ("System call ID is not valid!\n");
      thread_exit ();
    }
  }
  
}
