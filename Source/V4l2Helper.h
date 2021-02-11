/*=============================================================================
Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

Redistribution of this file, in original or modified form, without
prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

File:        V4l2Helper.h

Description:

-------------------------------------------------------------------------------

THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

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

    static std::string    ConvertResult2String(int result);
    static int xioctl(int fh, int request, void *arg);
    static std::string ConvertPixelformat2EnumString(int pixelformat);
    static std::string ConvertErrno2String(int errnumber);
    static std::string ConvertPixelformat2String(int pixelformat);
    static std::string ConvertControlID2String(uint32_t controlID);
};

} // namespace Examples
} // namespace Tools
} // namespace AVT

#endif /* V4L2HELPER_INCLUDE */
