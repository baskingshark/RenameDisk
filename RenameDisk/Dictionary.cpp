/**
 * Copyright (c) 2014, baskingshark
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <libkern/c++/OSSymbol.h>
#include <IOKit/IOLib.h>
#include "Dictionary.h"

#ifdef DEBUG
#define DLOG(fmt, ...) IOLog(fmt, ## __VA_ARGS__)
#else
#define DLOG(fmt, ...)
#endif

/*!
 * @class Callback
 *
 * @abstract
 * Callback provides an OSObject to store a function callback
 *
 * @discussion
 * This class is used to store callback functions (and an associated OSObject)
 * in an OSContainer.
 *
 * It is only intended to be used by the Dictionary class
 */
class Callback : public OSObject
{
    OSDeclareDefaultStructors(Callback);
public:
    /*!
     * @function withFunc
     *
     * @abstract
     * Create a Callback object for for the function specified
     *
     * @param f       A C function callback.
     * @param target  An OSObject passed to the callback on each call.  It is
     *                retained
     *
     * @result A new Callback with a retain count of 1 or NULL on failure
     */
    static Callback *withFunc(Dictionary::SetCallback  f,
                              const OSObject          *target)
    {
        if(!target || !f)
            return NULL;

        Callback *me = OSTypeAlloc(Callback);
        if(me) {
            if(me->init()) {
                me->f = f;
                me->target = target;
                if(target)
                    target->retain();
            }
            else
                OSSafeReleaseNULL(me);
        }
        return me;
    }
    /*!
     * @function free
     *
     * @abstract
     * Deallocates or releases any resources used by the instance
     *
     * @discussion
     * This function should not be called directly, use release instead
     */
    virtual void free()
    {
        OSSafeRelease(target);
        OSObject::free();
    }
    /*!
     * @function invoke the callback
     *
     * @abstract

     */
    virtual const OSMetaClassBase *invoke(const OSSymbol        *aKey,
                                          const OSMetaClassBase *aValue) const
    {
        return (*f)(target, aKey, aValue);
    }
private:
    Dictionary::SetCallback  f;
    const OSObject          *target;
};

// This required macro defines the class's constructors, destructors,
// and several other methods I/O Kit requires.
OSDefineMetaClassAndStructors(Callback, OSObject);

//
// Dictionary
//

// This required macro defines the class's constructors, destructors,
// and several other methods I/O Kit requires.
OSDefineMetaClassAndStructors(Dictionary, OSDictionary);

// Define the driver's superclass.
#define super OSDictionary

void Dictionary::free()
{
    OSSafeRelease(hooks);
    super::free();
}

bool Dictionary::setObject(const OSSymbol        *aKey,
                           const OSMetaClassBase *anObject)
{
    if(!aKey)
        return false;

    DLOG("%s[%p]::%s(%s, %p)\n",
         getMetaClass()->getClassName(), this, __FUNCTION__,
         aKey->getCStringNoCopy(), anObject);
    Callback *cb = static_cast<Callback*>(hooks->getObject(aKey));
    bool result;
    if(cb) {
        DLOG("%s[%p]::%s - invoking callback for '%s' object @ %p\n",
             getMetaClass()->getClassName(), this, __FUNCTION__,
             aKey->getCStringNoCopy(), anObject);
        cb->retain();
        anObject = cb->invoke(aKey, anObject);
        cb->release();
        DLOG("%s[%p]::%s - callback for '%s' returned object @ %p\n",
             getMetaClass()->getClassName(), this, __FUNCTION__,
             aKey->getCStringNoCopy(), anObject);
        result = super::setObject(aKey, anObject);
        OSSafeRelease(anObject);
    }
    else
        result = super::setObject(aKey, anObject);
    return result;
}

void Dictionary::removeObject(const OSSymbol *aKey)
{
    super::removeObject(aKey);
}

Dictionary *Dictionary::withDictionary(const OSDictionary *dict)
{
    DLOG("%s::%s(%p)\n",
         gMetaClass.getClassName(), __FUNCTION__, dict);
    if(!dict)
        return nullptr;

    Dictionary *me = OSTypeAlloc(Dictionary);
    if(me) {
        IOLog("%s::%s - created %s\n",
              gMetaClass.getClassName(),
              __FUNCTION__,
              gMetaClass.getClassName());
        me->hooks = OSDictionary::withCapacity(1);
        if(!me->hooks || !me->initWithDictionary(dict)) {
            IOLog("%s::%s - failed to init\n",
                  gMetaClass.getClassName(), __FUNCTION__);
            OSSafeRelease(me->hooks);
            OSSafeReleaseNULL(me);
        }
        else {
            DLOG("%s::%s - inited\n", gMetaClass.getClassName(), __FUNCTION__);
        }
    }

    return me;
}

bool Dictionary::addHook(const OSSymbol *aKey,
                         const OSObject *target,
                         SetCallback     cb
                         )
{
    if(!aKey)
        return false;

    DLOG("%s::%s('%s', %p, %p)\n",
         getMetaClass()->getClassName(), __FUNCTION__,
         aKey->getCStringNoCopy(), target, cb);
    Callback *c = Callback::withFunc(cb, target);
    if(c) {
        bool result = hooks->setObject(aKey, c);
        c->release();
        return result;
    }
    return false;
}

void Dictionary::removeHook(const OSSymbol *aKey)
{
    if(!aKey)
        return;
    DLOG("%s::%s('%s')\n",
         getMetaClass()->getClassName(), __FUNCTION__,
         aKey->getCStringNoCopy());
    hooks->removeObject(aKey);
}