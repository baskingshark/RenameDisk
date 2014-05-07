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

#include <IOKit/IOLib.h>
#include "RenameDisk.h"
#include "Dictionary.h"

#ifdef DEBUG
#define DLOG(fmt, ...) IOLog(fmt, ## __VA_ARGS__)
#else
#define DLOG(fmt, ...)
#endif

// This required macro defines the class's constructors, destructors,
// and several other methods I/O Kit requires.
OSDefineMetaClassAndStructors(NewIOBlockStorageDriver, IOBlockStorageDriver);

// Define the driver's superclass.
#define super IOBlockStorageDriver

// Type of device that we are going to mess with
// IOAHCIBlockStorageDriver is the base type for classes that talk to ACHI
// devices (IOAHCIDevice).  It interogates the disk and extracts properties such
// as "Model", "Revision", "Serial Number"
static const char *TARGET = "IOAHCIBlockStorageDriver";

// The property we want to change
static const char *MODEL = "Model";

// The prefix that we add to the model
static const char *PREFIX = "APPLE SSD";

/*!
 * @function fixModel
 *
 * @abstract
 * Change the model of a hard disk
 *
 * @discussion
 * This hook function is called whenever the Model number of a hard disk is
 * updated.  If needed, it adds the required prefix to fool the
 * IOAHCIBlockStorageDevice into believing that an Apple SSD is present.  This
 * will enable Trim support if the real drive supports Trim.
 *
 * @param target    A pointer to the driver
 * @param aKey      The name of the property being updated.  Should be "Model"
 * @param anObject  The new value for the Model.  This should be an OSString.
 *
 * @result The new Model, updated if needed
 */

static const OSMetaClassBase*
fixModel(const OSObject        *target,
         const OSSymbol        *aKey,
         const OSMetaClassBase *anObject)
{
    const OSMetaClassBase *result = anObject;
    const OSString *realModel = OSDynamicCast(OSString, anObject);
    if(result)
        result->retain();
    if(realModel) {
        const char *realModelCStr = realModel->getCStringNoCopy();
        if(strncmp(PREFIX, realModelCStr, strlen(PREFIX))) {
            int required = snprintf(NULL, 0, "%s (%s)", PREFIX, realModelCStr);
            if(required >= 0) {
                required++; // Add extra space for NULL
                char *buffer = static_cast<char*>(IOMalloc(required));
                if(buffer) {
                    snprintf(buffer, required,
                             "%s (%s)", PREFIX, realModelCStr);
                    const OSString *newModel = OSString::withCString(buffer);
                    DLOG("%s[%p]::%s - Changing '%s' from '%s' to '%s'\n",
                         target->getMetaClass()->getClassName(), target,
                         __FUNCTION__,
                         aKey->getCStringNoCopy(), realModelCStr, buffer);
                    OSSafeRelease(result);
                    result = newModel;
                    IOFree(buffer, required);
                }
                else
                    IOLog("%s[%p]::%s - Failed to allocate temporary buffer\n",
                          target->getMetaClass()->getClassName(),
                          target, __FUNCTION__);
            }
            else
                IOLog("%s[%p]::%s - Failed to determine size of buffer\n",
                      target->getMetaClass()->getClassName(),
                      target, __FUNCTION__);
        }
        else
            DLOG("%s[%p]::%s - Prefix found ... not updating\n",
                 target->getMetaClass()->getClassName(), target, __FUNCTION__);
    }
    else
        IOLog("%s[%p]::%s - Value is not a string ... cannot update\n",
              target->getMetaClass()->getClassName(), target, __FUNCTION__);
    return result;
}

#ifdef DEBUG
#define DLOGDICT(prefix,dict,suffix)\
do {\
    OSSerialize *ser = OSSerialize::withCapacity(4096);\
    if(ser) {\
        (dict)->serialize(ser);\
        DLOG("%s%s%s", (prefix), ser->text(), (suffix));\
        ser->release();\
    }\
} while(0)
#else
#define DLOGDICT(prefix,dict,suffix)
#endif

/*!
 * @function hookProperties
 *
 * @abstract
 * Replace the property table with a hookable Dictionary
 *
 * @discussion
 * This function replaces the property table on the target object with a custom
 * Dictionary and hooks changes to the "Model" property.  Any changes are
 * signalled by a call to the fixModel method.
 *
 * This should only be called within a call to IOService::runPropertyAction()
 *
 * @param target  A pointer to self
 * @param arg0    An IOService object to hook
 * @param arg1    Unused
 * @param arg2    Unused
 * @param arg3    Unused
 */
static
IOReturn
hookProperties(OSObject *target,
               void     *arg0,
               void     *arg1,
               void     *arg2,
               void     *arg3)
{
    NewIOBlockStorageDriver *me =
        OSDynamicCast(NewIOBlockStorageDriver, target);
    IOService *tgt = (IOService*) arg0;
    IOReturn result;

    if(!me || !tgt)
        return kIOReturnInternalError;

    DLOG("%s[%p]::%s(%p, %p)\n", me->getName(), me, __FUNCTION__, me, tgt);
    OSDictionary *cur = tgt->getPropertyTable();
    if(cur) {
        DLOG("%s[%p]::%s - Current property table @ %p",
             me->getName(), me, __FUNCTION__, cur);
        DLOGDICT(" = ", cur, "");
        DLOG("\n");
        // Check whether dictionary has already been replaced before creating
        Dictionary *propTable = OSDynamicCast(Dictionary, cur);
        if(propTable) {
            propTable->retain();
        }
        else {
            propTable = Dictionary::withDictionary(cur);
        }
        if(propTable) {
            DLOG("%s[%p]::%s - New property table @ %p",
                 me->getName(), me, __FUNCTION__, propTable);
            DLOGDICT(" = ", cur, "");
            DLOG("\n");

            // Add hook
            const OSSymbol *model = OSSymbol::withCStringNoCopy(MODEL);
            if(model) {
                propTable->addHook(model, target, fixModel);
                model->release();
                tgt->setPropertyTable(propTable);
                result = kIOReturnSuccess;
            }
            else {
                IOLog("%s[%p]::%s - Failed to add hook for '%s'\n",
                      me->getName(), me, __FUNCTION__, MODEL);
                result = kIOReturnNoMemory;
            }
            propTable->release();
        }
        else {
            IOLog("%s[%p]::%s - Failed to create new Dictionary\n",
                  me->getName(), me, __FUNCTION__);
            result = kIOReturnNoMemory;
        }
    }
    else {
        IOLog("%s[%p]::%s - Failed to get property table for %s @ %p\n",
              me->getName(), me, __FUNCTION__, tgt->getName(), tgt);
        result = kIOReturnInternalError;
    }

    return result;
}

/*!
 * @function unhookProperties
 *
 * @abstract
 * Remove a hookable Dictionary are replace it with a standard OSDictionary
 *
 * @discussion
 * This function replaces the property table on the target object with a
 * standard OSDictionary.
 *
 * This should only be called within a call to IOService::runPropertyAction()
 *
 * @param target  A pointer to self
 * @param arg0    An IOService object to unhook
 * @param arg1    Unused
 * @param arg2    Unused
 * @param arg3    Unused
 */
static
IOReturn
unhookProperties(OSObject *target,
                 void     *arg0,
                 void     *arg1,
                 void     *arg2,
                 void     *arg3)
{
    NewIOBlockStorageDriver *me =
        OSDynamicCast(NewIOBlockStorageDriver, target);
    IOService *tgt = static_cast<IOService*>(arg0);

    if(!me || !tgt)
        return kIOReturnInternalError;

    DLOG("%s[%p]::%s(%p, %p)\n", me->getName(), me, __FUNCTION__, me, tgt);

    // Check that the current property table is one of ours.
    Dictionary *cur = OSDynamicCast(Dictionary, tgt->getPropertyTable());
    if(cur) {
        DLOG("%s[%p]::%s - Replacing current property table @ %p",
             me->getName(), me, __FUNCTION__, cur);
        DLOGDICT(" = ", cur, "");
        DLOG("\n");
        OSDictionary *newDict = OSDictionary::withDictionary(cur);
        if(newDict) {
            DLOG("%s[%p]::%s - New property table @ %p",
                 me->getName(), me, __FUNCTION__, newDict);
            DLOGDICT(" = ", newDict, "");
            DLOG("\n");
            // setPropertyTable release current Dictionary when a new one is set
            // setPropertyTable retains new Dictionary when a new one is set
            tgt->setPropertyTable(newDict);
            newDict->release();
        }
        else
            IOLog("%s[%p]::%s - Failed to allocate replacement OSDictionary\n",
                  me->getName(), me, __FUNCTION__);
    }
    else {
        IOLog("%s[%p]::%s - Failed to get property table for %s @ %p\n",
              me->getName(), me, __FUNCTION__, tgt->getName(), tgt);
        return kIOReturnInternalError;
    }
    return kIOReturnSuccess;
}

/*!
 * @function getTargetService
 *
 * @abstract
 * Locate the target IOService
 *
 * @discussion
 * This function traverses the IOService tree looking for an instance of TARGET
 *
 * @param me  A pointer to self
 *
 * @result The target IOService, or NULL if it was not found
 */
static
IOService *getTargetService(IOService *me)
{
    DLOG("%s[%p]::%s()\n", me->getName(), me, __FUNCTION__);

    const OSSymbol *tgtName = OSSymbol::withCStringNoCopy(TARGET);
    IOService *result = NULL;
    if(tgtName) {
        const OSMetaClass *tgtClass =
            me->getMetaClass()->getMetaClassWithName(tgtName);
        if(tgtClass) {
            IOService *root = me->getServiceRoot();
            IOService *p = me->getProvider();
            while(p && p != root) {
                DLOG("%s[%p]::%s - Got %s[%p]",
                     me->getName(), me, __FUNCTION__, p->getName(), p);
                if(tgtClass->checkMetaCast(p)) {
                    DLOG(" - SUCCESS\n");
                    result = p;
                    break;
                }
                else {
                    DLOG(" - SKIP\n");
                    p = p->getProvider();
                }
            }
        }
        else
            IOLog("%s[%p]::%s - failed to get OSMetaClass for '%s'\n",
                  me->getName(), me, __FUNCTION__, TARGET);
        tgtName->release();
    }
    else
        IOLog("%s[%p]::%s - failed to create OSSymbol for '%s'\n",
              me->getName(), me, __FUNCTION__, TARGET);

    return result;
}

IOService *NewIOBlockStorageDriver::probe(IOService *provider,
                                          SInt32    *score)
{
    DLOG("%s[%p]::%s(%p, %d)\n",
         getName(), this, __FUNCTION__, provider, *score);
    IOService *result = super::probe(provider, score);
    if(result && !getTargetService(this))
        result = NULL;
    return result;
}

bool NewIOBlockStorageDriver::start(IOService *provider)
{
    DLOG("%s[%p]::%s(%p)\n", getName(), this, __FUNCTION__, provider);
    IOService *tgt = getTargetService(this);
    if(tgt) {
        if(OSDynamicCast(Dictionary, tgt->getPropertyTable())) {
            DLOG("%s[%p]::%s - target (%s) is already hooked ... skipping\n",
                 getName(), this, __FUNCTION__, tgt->getName());
        }
        else {
            IOService *tgtParent = tgt->getProvider();
            tgt->retain();
            // Target has opened provider ... close it
            if(tgtParent->isOpen(tgt)) {
                DLOG("%s[%p]::%s - %s[%p] has opened %s[%p] ... closing\n",
                     getName(), this, __FUNCTION__,
                     tgt->getName(), tgt, tgtParent->getName(), tgtParent);
                tgtParent->close(tgt);
            }
            // Stop target
            DLOG("%s[%p]::%s - Stopping %s[%p]\n",
                 getName(), this, __FUNCTION__, tgt->getName(), tgt);
            tgt->stop(tgtParent);
            // Hook dictionary on target
            DLOG("%s[%p]::%s - patching property dict on %s[%p]\n",
                 getName(), this, __FUNCTION__, tgt->getName(), tgt);
            tgt->runPropertyAction(hookProperties, this, tgt);
            // Restart target
            DLOG("%s[%p]::%s - Restarting %s[%p] ... ",
                 getName(), this, __FUNCTION__, tgt->getName(), tgt);
            bool result = tgt->start(tgtParent);
            DLOG("%s\n", result ? "OK" : "FAILED");
            tgt->release();

            // Terminate all services between us and the target
            IOService *p = provider;
            while(p != tgt) {
                IOService *pParent = p->getProvider();
                DLOG("%s[%p]::%s - terminating %s[%p] ... ",
                     getName(), this, __FUNCTION__, p->getName(), p);
                bool result = p->terminate();
                DLOG("%s\n", result ? "OK" : "FAILED");
                p = pParent;
            }
            // Restarting target should have created a new device tree
            // We have also terminated our parents, grandparents, ...
            // So, fail to start
            return false;
        }
    }
    else
        IOLog("%s[%p]::%s - target (%s) not found\n",
              getName(), this, __FUNCTION__, TARGET);
    return super::start(provider);
}

void NewIOBlockStorageDriver::stop(IOService *provider)
{
    DLOG("%s[%p]::%s(%p)\n", getName(), this, __FUNCTION__, provider);
    IOService *tgt = getTargetService(this);
    if(tgt) {
        // Unhook dictionary on target
        DLOG("%s[%p]::%s - unpatching property dict on %s[%p]\n",
             getName(), this, __FUNCTION__, tgt->getName(), tgt);
        tgt->retain();
        tgt->runPropertyAction(unhookProperties, this, tgt);
        tgt->release();
    }
    else
        IOLog("%s[%p]::%s - target (%s) not found\n",
              getName(), this, __FUNCTION__, TARGET);
    return super::stop(provider);
}