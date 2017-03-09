/*
 * pubsub.h
 *
 *  Created on: 24 févr. 2017
 *      Author: jfellus
 */

#ifndef DDBUS_H_
#define DDBUS_H_


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @param callback : a pointer to function that will be called upon incoming messages on the bus
 * @return : A file descriptor to write on the bus
 * */
int ddbus_open(void (*callback) (const char*));



#ifdef __cplusplus
}
#endif

#endif /* DDBUS_H_ */
