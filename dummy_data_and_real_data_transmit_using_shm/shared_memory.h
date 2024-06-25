/*
 * shared_memory.h
 *
 *  Created on: May 15, 2024
 *      Author: Thusitha Samarasekara
 */

#ifndef SHARED_MEMORY_H_
#define SHARED_MEMORY_H_

extern int shared_memory_init(void);

extern int shared_memory_release(void);

extern char *shared_memory_get_addr(void);

#endif /* SHARED_MEMORY_H_ */
