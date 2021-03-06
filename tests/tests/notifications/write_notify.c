#include <test_utils.h>

int
main (int argc, char *argv[])
{
  TSUITE_INIT (argc, argv);

  ASSERT (gaspi_proc_init (GASPI_BLOCK));

  gaspi_notification_id_t n = 0;
  gaspi_rank_t rank, nprocs, i;
  const gaspi_segment_id_t seg_id = 0;
  gaspi_offset_t offset;

  gaspi_number_t queue_size;
  gaspi_number_t queue_max;

  ASSERT (gaspi_queue_size_max (&queue_max));

  ASSERT (gaspi_proc_num (&nprocs));
  ASSERT (gaspi_proc_rank (&rank));

  if (nprocs < 2)
  {
    return EXIT_SUCCESS;
  }

  ASSERT (gaspi_segment_create
          (seg_id, nprocs * sizeof (int), GASPI_GROUP_ALL, GASPI_BLOCK,
           GASPI_MEM_UNINITIALIZED));

  offset = rank * sizeof (int);

  gaspi_pointer_t _vptr;

  ASSERT (gaspi_segment_ptr (0, &_vptr));

  int *mem = (int *) _vptr;

  for (i = 0; i < nprocs; i++)
  {
    mem[i] = (int) rank;
  }

  ASSERT (gaspi_barrier (GASPI_GROUP_ALL, GASPI_BLOCK));

  for (i = 0; i < nprocs; i++)
  {
    ASSERT (gaspi_queue_size (0, &queue_size));
    if (queue_size > queue_max - 1)
      ASSERT (gaspi_wait (0, GASPI_BLOCK));

    ASSERT (gaspi_write_notify (seg_id, offset, i,
                                seg_id, offset, sizeof (int),
                                (gaspi_notification_id_t) rank, 1,
                                0, GASPI_BLOCK));
  }

  do
  {
    gaspi_notification_id_t id;

    ASSERT (gaspi_notify_waitsome
            (seg_id, 0, (gaspi_notification_id_t) nprocs, &id, GASPI_BLOCK));

    gaspi_notification_t notification_val;

    ASSERT (gaspi_notify_reset (seg_id, id, &notification_val));

    assert (notification_val == 1);
    n++;
  }
  while (n < (nprocs));

  ASSERT (gaspi_wait (0, GASPI_BLOCK));

  //check
  for (i = 0; i < nprocs; i++)
  {
    assert (mem[i] == i);
  }

  ASSERT (gaspi_barrier (GASPI_GROUP_ALL, GASPI_BLOCK));
  ASSERT (gaspi_proc_term (GASPI_BLOCK));

  return EXIT_SUCCESS;
}
