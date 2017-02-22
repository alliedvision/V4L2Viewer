#ifndef HELPER_INCLUDE
#define HELPER_INCLUDE

#include <stdint.h>
#include <string>

namespace AVT {
namespace BaseTools {

    class Helper
    {
    public:
        Helper(void);
        virtual ~Helper(void);

        static void SwapByteOrder(void *pData, size_t nSize);

        static std::string ConvertCommandID2String(uint16_t command);
        static std::string ConvertPixelformat2String(uint32_t nPixelFormat);

        static std::string ConvertPacket2String(uint8_t* buffer, uint32_t length);
        
        static void OutputDebugWindow(const char *text, ...);

        static std::string VSPrint(const char *text, ...);

        static void uSleep(uint32_t msec);

        static std::string GetTimeStamp();
    
    };

}} /* namespace AVT::BaseTools { */

#endif /* HELPER_INCLUDE */
