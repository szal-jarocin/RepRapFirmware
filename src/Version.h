/*
 * Version.h
 *
 *  Created on: 25 Dec 2016
 *      Author: David
 */

#ifndef SRC_VERSION_H_
#define SRC_VERSION_H_

#ifndef VERSION
# define MAIN_VERSION	"3.02-beta"
# ifdef USE_CAN0
#  define VERSION_SUFFIX	" (CAN0)"
# else
#  define VERSION_SUFFIX	""
# endif
# define VERSION MAIN_VERSION VERSION_SUFFIX
#endif

#ifndef DATE
<<<<<<< HEAD
# define DATE "2020-06-27b1"
=======
# define DATE "2020-9-8b15"
>>>>>>> v3.01-dev-lpc
#endif

#define AUTHORS "reprappro, dc42, chrishamm, t3p3, dnewman, printm3d"

#endif /* SRC_VERSION_H_ */
