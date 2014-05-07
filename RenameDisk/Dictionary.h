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

#ifndef __Dictionary__
#define __Dictionary__

#include <libkern/c++/OSDictionary.h>

#define Dictionary baskingshark_Dictionary

class Dictionary : public OSDictionary {
    OSDeclareDefaultStructors(Dictionary);
public:
    /*!
     * @function withDictionary
     *
     * @abstract
     * Create a Dictionary based on the one provided
     *
     * @param dict  The reference dictionary (must not be NULL)
     *
     * @result
     * A new Dictionary with a retain count of 1 or NULL on failure
     */
    static Dictionary *withDictionary(const OSDictionary *dict);

    /*!
     * @function free
     *
     * @abstract
     * Deallocates or releases any resources used by the instance
     *
     * @discussion
     * This function should not be called directly, use release instead
     */
    virtual void free();

    /*!
     * @function setObject
     *
     * @abstract
     * Stores an object in the dictionary under a key.
     *
     * @param aKey      An OSSymbol identifying the object placed within the
     *                  dictionary.  It is automatically retained.
     * @param anObject  The object to be stored in the dictionary.  It is
     *                  automatically retained.
     *
     * @result
     * true or false depending on whether the addition was successful or not
     */
    virtual bool setObject(const OSSymbol        *aKey,
                           const OSMetaClassBase *anObject);

    /*!
     * @function removeObject
     *
     * @abstract
     * Removes a key/object pair from the dictionary.
     *
     * @param aKey  An OSSymbol identifying the object to be removed from the
     *              dictionary.
     */
    virtual void removeObject(const OSSymbol *aKey);

    /*!
     * @typedef SetCallback
     *
     * @param target    Reference supplied when the callback was registered.
     * @param aKey      An OSSymbol identifying the object placed within the
     *                  dictionary.
     * @param anObject  The object the user requested to be stored in the
     *                  dictionary.
     *
     * @result
     * The actual object to be stored in the dictionary.  This is released
     * after adding to the dictionary.  If anObject is to be added, it must be
     * retained before return.
     */
    typedef
    const OSMetaClassBase* (*SetCallback)(const OSObject        *target,
                                          const OSSymbol        *aKey,
                                          const OSMetaClassBase *anObject);

    /*!
     * @function addHook
     *
     * @abstract
     * Set hook function for the given key.
     *
     * @param aKey    An OSSymbol identifying an object within the dictionary.
     *                It is automatically retained.
     * @param target  An OSObject passed to the callback on each call.  It is
     *                retained.
     * @param setCB   A C function callback to be called whenever aKey is
     *                updated.
     */
    virtual bool addHook(const OSSymbol *aKey,
                         const OSObject *target,
                         SetCallback     setCB);
    /*!
     * @function removeHook
     *
     * @abstract
     * Remove any hook function for the given key.
     *
     * @param aKey  An OSSymbol identifying an object within the dictionary.
     */
    virtual void removeHook(const OSSymbol *aKey);
private:
    OSDictionary *hooks;
};

#endif /* defined(__Dict__) */
