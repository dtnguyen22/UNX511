//keyGenerator.h - header file for the key generator
//
// 11-Feb-21  M. Watler         Created
//

//Question 31: What does _IOW indicate about this ioctl?
//answer: ioctl with write parameters (copy_from_user)
#define KEY_GENERATOR_CHANGE_KEY _IOW('a','b',int*)
