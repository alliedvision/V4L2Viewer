/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FrameObserver.h

  Description: The frame observer that is used for notifications
               regarding the arrival of a newly acquired frame.

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

#ifndef FRAMEOBSERVERMMAP_INCLUDE
#define FRAMEOBSERVERMMAP_INCLUDE

#include "Helper.h"
#include "V4l2Helper.h"

#include <FrameObserver.h>


////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////


namespace AVT {
namespace Tools {
namespace Examples {

class FrameObserverMMAP : public FrameObserver
{
  public:
    // We pass the camera that will deliver the frames to the constructor
    FrameObserverMMAP();
    //
    virtual ~FrameObserverMMAP();

    virtual int CreateUserBuffer(uint32_t bufferCount, uint32_t bufferSize);
    virtual int QueueAllUserBuffer();
    virtual int QueueSingleUserBuffer(const int index);
    virtual int DeleteUserBuffer();
    
protected:
    // v4l2
    virtual int ReadFrame();
	
private:

};

}}} // namespace AVT::Tools::Examples

#endif /* FRAMEOBSERVER_INCLUDE */
