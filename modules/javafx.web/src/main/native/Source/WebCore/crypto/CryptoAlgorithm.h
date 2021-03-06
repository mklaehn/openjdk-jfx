/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "CryptoAlgorithmIdentifier.h"
#include "CryptoKeyFormat.h"
#include "CryptoKeyPair.h"
#include "CryptoKeyUsage.h"
#include "ExceptionOr.h"
#include "JsonWebKey.h"
#include "SubtleCrypto.h"
#include <wtf/Function.h>
#include <wtf/Variant.h>
#include <wtf/Vector.h>
#include <wtf/WorkQueue.h>

#if ENABLE(SUBTLE_CRYPTO)

namespace WebCore {

class CryptoAlgorithmParameters;
class CryptoKey;
class ScriptExecutionContext;

using KeyData = Variant<Vector<uint8_t>, JsonWebKey>;
using KeyOrKeyPair = Variant<RefPtr<CryptoKey>, CryptoKeyPair>;

class CryptoAlgorithm : public RefCounted<CryptoAlgorithm> {
public:
    virtual ~CryptoAlgorithm() = default;

    virtual CryptoAlgorithmIdentifier identifier() const = 0;

    using BoolCallback = WTF::Function<void(bool)>;
    using KeyCallback = WTF::Function<void(CryptoKey&)>;
    using KeyOrKeyPairCallback = WTF::Function<void(KeyOrKeyPair&&)>;
    // FIXME: https://bugs.webkit.org/show_bug.cgi?id=169395
    using VectorCallback = WTF::Function<void(const Vector<uint8_t>&)>;
    using VoidCallback = WTF::Function<void()>;
    using ExceptionCallback = WTF::Function<void(ExceptionCode)>;
    using KeyDataCallback = WTF::Function<void(CryptoKeyFormat, KeyData&&)>;

    virtual void encrypt(std::unique_ptr<CryptoAlgorithmParameters>&&, Ref<CryptoKey>&&, Vector<uint8_t>&&, VectorCallback&&, ExceptionCallback&&, ScriptExecutionContext&, WorkQueue&);
    virtual void decrypt(std::unique_ptr<CryptoAlgorithmParameters>&&, Ref<CryptoKey>&&, Vector<uint8_t>&&, VectorCallback&&, ExceptionCallback&&, ScriptExecutionContext&, WorkQueue&);
    virtual void sign(std::unique_ptr<CryptoAlgorithmParameters>&&, Ref<CryptoKey>&&, Vector<uint8_t>&&, VectorCallback&&, ExceptionCallback&&, ScriptExecutionContext&, WorkQueue&);
    virtual void verify(std::unique_ptr<CryptoAlgorithmParameters>&&, Ref<CryptoKey>&&, Vector<uint8_t>&& signature, Vector<uint8_t>&&, BoolCallback&&, ExceptionCallback&&, ScriptExecutionContext&, WorkQueue&);
    virtual void digest(Vector<uint8_t>&&, VectorCallback&&, ExceptionCallback&&, ScriptExecutionContext&, WorkQueue&);
    virtual void generateKey(const CryptoAlgorithmParameters&, bool extractable, CryptoKeyUsageBitmap, KeyOrKeyPairCallback&&, ExceptionCallback&&, ScriptExecutionContext&);
    virtual void deriveBits(std::unique_ptr<CryptoAlgorithmParameters>&&, Ref<CryptoKey>&&, size_t length, VectorCallback&&, ExceptionCallback&&, ScriptExecutionContext&, WorkQueue&);
    // FIXME: https://bugs.webkit.org/show_bug.cgi?id=169262
    virtual void importKey(CryptoKeyFormat, KeyData&&, const std::unique_ptr<CryptoAlgorithmParameters>&&, bool extractable, CryptoKeyUsageBitmap, KeyCallback&&, ExceptionCallback&&);
    virtual void exportKey(CryptoKeyFormat, Ref<CryptoKey>&&, KeyDataCallback&&, ExceptionCallback&&);
    virtual void wrapKey(Ref<CryptoKey>&&, Vector<uint8_t>&&, VectorCallback&&, ExceptionCallback&&);
    virtual void unwrapKey(Ref<CryptoKey>&&, Vector<uint8_t>&&, VectorCallback&&, ExceptionCallback&&);
    virtual ExceptionOr<size_t> getKeyLength(const CryptoAlgorithmParameters&);

    static void dispatchOperation(WorkQueue&, ScriptExecutionContext&, VectorCallback&&, ExceptionCallback&&, WTF::Function<ExceptionOr<Vector<uint8_t>>()>&&);
    static void dispatchOperation(WorkQueue&, ScriptExecutionContext&, BoolCallback&&, ExceptionCallback&&, WTF::Function<ExceptionOr<bool>()>&&);
};

} // namespace WebCore

#endif // ENABLE(SUBTLE_CRYPTO)
