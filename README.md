# Circular Buffer implementation with mmap(2)

This is a [circular buffer](https://en.wikipedia.org/wiki/Circular_buffer)
implementation using mmap to ease the logic behind the caller which
needs to know if the polled item crosses the maximum size of the
underlying array.

## Internals

1. mmap is used to mirror the buffer like the following:
![cirbuf_mmap](docs/circular_buffer_mmap.png)

2. the "mirrored" buffer is then placed beside the buffer.
   When the user polls the item it doesn't matter if the item crosses the buffer's boundary:
![cirbuf_mmap_portal](docs/circular_buffer_mmap_portal.png)
