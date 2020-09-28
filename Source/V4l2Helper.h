#ifndef V4L2HELPER_INCLUDE
#define V4L2HELPER_INCLUDE

#include <stdint.h>
#include <string>

namespace AVT {
namespace Tools {
namespace Examples {

    class V4l2Helper
    {
    public:
        V4l2Helper(void);
        virtual ~V4l2Helper(void);

        static std::string	ConvertResult2String(int result);
		static int xioctl(int fh, int request, void *arg);
		static std::string ConvertPixelformat2EnumString(int pixelformat);
		static std::string ConvertErrno2String(int errnumber);
		static std::string ConvertPixelformat2String(int pixelformat);
		static std::string ConvertControlID2String(uint32_t controlID);

    };

}}} /* namespace AVT::Tools::Examples { */

#endif /* V4L2HELPER_INCLUDE */
