/*
 * Copyright (C) 2022 The Android Open Source Project
 *               2025 anonymix007
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include <cstddef>

#define LOG_TAG "JamesDSP_AIDL"
#include <android-base/logging.h>

#include <aidl/android/hardware/audio/effect/DefaultExtension.h>
#include <fmq/AidlMessageQueue.h>
#include <media/AidlConversionNdk.h>
#include <system/audio_effects/effect_uuid.h>

#include "jamesdsp_aidl.h"

using aidl::android::hardware::audio::effect::Descriptor;
using aidl::android::hardware::audio::effect::DefaultExtension;
using aidl::android::hardware::audio::effect::JamesDSPAIDL;
using aidl::android::hardware::audio::effect::getEffectTypeUuidCustom;
using aidl::android::hardware::audio::effect::getEffectImplUuidJDSP;
using aidl::android::hardware::audio::effect::getEffectUuidNull;
using aidl::android::hardware::audio::effect::IEffect;
using aidl::android::hardware::audio::effect::State;
using aidl::android::hardware::audio::effect::VendorExtension;
using aidl::android::media::audio::common::AudioUuid;

extern "C" binder_exception_t createEffect(const AudioUuid* in_impl_uuid,
                                           std::shared_ptr<IEffect>* instanceSpp) {
    if (!in_impl_uuid || *in_impl_uuid != getEffectImplUuidJDSP()) {
        LOG(ERROR) << __func__ << "uuid not supported";
        return EX_ILLEGAL_ARGUMENT;
    }
    if (instanceSpp) {
        *instanceSpp = ndk::SharedRefBase::make<JamesDSPAIDL>();
        LOG(DEBUG) << __func__ << " instance " << instanceSpp->get() << " created";
        return EX_NONE;
    } else {
        LOG(ERROR) << __func__ << " invalid input parameter!";
        return EX_ILLEGAL_ARGUMENT;
    }
}

extern "C" binder_exception_t queryEffect(const AudioUuid* in_impl_uuid, Descriptor* _aidl_return) {
    if (!in_impl_uuid || *in_impl_uuid != getEffectImplUuidJDSP()) {
        LOG(ERROR) << __func__ << "uuid not supported";
        return EX_ILLEGAL_ARGUMENT;
    }
    *_aidl_return = JamesDSPAIDL::kDesc;
    return EX_NONE;
}

namespace aidl::android::hardware::audio::effect {

#if __aarch64__ == 1
const std::string JamesDSPAIDL::kEffectName = "James Audio DSP arm64";
#elif __ARM_ARCH_7A__ == 1
const std::string JamesDSPAIDL::kEffectName = "James Audio DSP arm32";
#elif __i386__ == 1
const std::string JamesDSPAIDL::kEffectName = "James Audio DSP x86";
#elif __x86_64__ == 1
const std::string JamesDSPAIDL::kEffectName = "James Audio DSP x64";
#endif

const Descriptor JamesDSPAIDL::kDesc = {.common = {.id = {.type = getEffectTypeUuidCustom(),
                                                          .uuid = getEffectImplUuidJDSP()},
                                                  .flags = {.type = Flags::Type::INSERT,
                                                            .insert = Flags::Insert::LAST,
                                                            .volume = Flags::Volume::CTRL},
                                                  .name = JamesDSPAIDL::kEffectName,
                                                  .implementor = "James Fung"}};

ndk::ScopedAStatus JamesDSPAIDL::getDescriptor(Descriptor* _aidl_return) {
    LOG(DEBUG) << __func__ << kDesc.toString();
    *_aidl_return = kDesc;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus JamesDSPAIDL::setParameterSpecific(const Parameter::Specific& specific) {
    RETURN_IF(!mContext, EX_NULL_POINTER, "nullContext");
    RETURN_IF(Parameter::Specific::vendorEffect != specific.getTag(), EX_ILLEGAL_ARGUMENT, "EffectNotSupported");
    auto& vendorEffect = specific.get<Parameter::Specific::vendorEffect>();
    std::optional<DefaultExtension> defaultExt;
    RETURN_IF(STATUS_OK != vendorEffect.extension.getParcelable(&defaultExt), EX_ILLEGAL_ARGUMENT, "getParcelableFailed");
    RETURN_IF(!defaultExt.has_value(), EX_ILLEGAL_ARGUMENT, "parcelableNull");

#ifdef AIDL_DEBUG
    LOG(DEBUG) << __func__ << ": defaultExt: " << defaultExt->toString();
#endif

    int32_t ret = 0;
    uint32_t ret_size = sizeof(ret);
    RETURN_IF(mContext->handleCommand(EFFECT_CMD_SET_PARAM, defaultExt->bytes.size(), defaultExt->bytes.data(), &ret_size, &ret) != 0, EX_ILLEGAL_ARGUMENT, "handleCommandFailed");
    RETURN_IF(ret != 0, EX_ILLEGAL_ARGUMENT, "handleCommandInternalFailed");
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus JamesDSPAIDL::getParameterSpecific(const Parameter::Id& id,
                                                     Parameter::Specific* specific) {
    RETURN_IF(!mContext, EX_NULL_POINTER, "nullContext");
    RETURN_IF(Parameter::Id::vendorEffectTag != id.getTag(), EX_ILLEGAL_ARGUMENT, "wrongIdTag");
    auto extensionId = id.get<Parameter::Id::vendorEffectTag>();
    std::optional<DefaultExtension> defaultIdExt;
    RETURN_IF(STATUS_OK != extensionId.extension.getParcelable(&defaultIdExt), EX_ILLEGAL_ARGUMENT, "getIdParcelableFailed");
    RETURN_IF(!defaultIdExt.has_value(), EX_ILLEGAL_ARGUMENT, "parcelableIdNull");

#ifdef AIDL_DEBUG
    LOG(DEBUG) << __func__ << ": defaultIdExt: " << defaultIdExt->toString();
#endif

    VendorExtension extension;
    DefaultExtension defaultExt;
    defaultExt.bytes.resize(sizeof(effect_param_t) + 2 * sizeof(int32_t));
    uint32_t data_size = defaultExt.bytes.size();
    RETURN_IF(mContext->handleCommand(EFFECT_CMD_GET_PARAM, defaultIdExt->bytes.size(), defaultIdExt->bytes.data(), &data_size, defaultExt.bytes.data()) != 0, EX_ILLEGAL_ARGUMENT, "handleCommandFailed");
    assert(data_size <= defaultExt.bytes.size());
    defaultExt.bytes.resize(data_size);

#ifdef AIDL_DEBUG
    LOG(DEBUG) << __func__ << ": defaultExt: " << defaultExt.toString();
#endif

    RETURN_IF(STATUS_OK != extension.extension.setParcelable(defaultExt), EX_ILLEGAL_ARGUMENT, "setParcelableFailed");
    specific->set<Parameter::Specific::vendorEffect>(extension);
    return ndk::ScopedAStatus::ok();
}

static inline std::string buffer_config_to_string(const buffer_config_t &conf)  {
    std::ostringstream stream_out;
    stream_out << "rate: " << conf.samplingRate << ", channels: " << conf.channels << ", format: " << (int) conf.format;
    return stream_out.str();
}

std::shared_ptr<EffectContext> JamesDSPAIDL::createContext(const Parameter::Common& common) {
    if (mContext) {
        LOG(DEBUG) << __func__ << " context already exists";
    } else {
        mContext = std::make_shared<JamesDSPAIDLContext>(1 /* statusFmqDepth */, common);
        int32_t ret = 0;
        uint32_t ret_size = sizeof(ret);
        int32_t status = mContext->handleCommand(EFFECT_CMD_INIT, 0, NULL, &ret_size, &ret);
        if (status < 0) {
            LOG(ERROR) << __func__ << ": JamesDSPAIDLContext::handleCommand INIT failed: " << status;
            mContext = nullptr;
            return mContext;
        }
        if (ret < 0) {
            LOG(ERROR) << __func__ << ": JamesDSPAIDLContext::handleCommand INIT failed (internal): " << ret;
            mContext = nullptr;
            return mContext;
        }
        ret = 0;
        ret_size = sizeof(ret);
        effect_config_t conf;

        if (!aidl2legacy_ParameterCommon_effect_config_t(common, conf)) {
            LOG(ERROR) << __func__ << ": JamesDSPAIDL::createContext: aidl2legacy_ParameterCommon_effect_config_t failed";
            mContext = nullptr;
            return mContext;
        }

        LOG(INFO) << __func__ << ": AIDL input config: " << common.input.toString();
        LOG(INFO) << __func__ << ": AIDL output config: " << common.output.toString();
        LOG(INFO) << __func__ << ": Legacy input config: " << buffer_config_to_string(conf.inputCfg);
        LOG(INFO) << __func__ << ": Legacy output config: " << buffer_config_to_string(conf.outputCfg);

        status = mContext->handleCommand(EFFECT_CMD_SET_CONFIG, sizeof(conf), &conf, &ret_size, &ret);
        if (status < 0) {
            LOG(ERROR) << __func__ << ": JamesDSPAIDLContext::handleCommand SET CONFIG failed: " << status;
            mContext = nullptr;
            return mContext;
        }
        if (ret < 0) {
            LOG(ERROR) << __func__ << ": JamesDSPAIDLContext::handleCommand SET CONFIG failed (internal): " << ret;
            mContext = nullptr;
            return mContext;
        }
    }

    return mContext;
}

ndk::ScopedAStatus JamesDSPAIDL::commandImpl(CommandId command) {
    RETURN_IF(!mImplContext, EX_NULL_POINTER, "nullContext");
    int32_t ret = 0;
    uint32_t ret_size = sizeof(ret);
    switch (command) {
        case CommandId::START:
            RETURN_IF(mContext->handleCommand(EFFECT_CMD_ENABLE, 0, NULL, &ret_size, &ret) != 0, EX_ILLEGAL_ARGUMENT, "handleCommandFailed");
            RETURN_IF(ret != 0, EX_ILLEGAL_ARGUMENT, "handleCommandInternalFailed");
            break;
        case CommandId::STOP:
            RETURN_IF(mContext->handleCommand(EFFECT_CMD_DISABLE, 0, NULL, &ret_size, &ret) != 0, EX_ILLEGAL_ARGUMENT, "handleCommandFailed");
            RETURN_IF(ret != 0, EX_ILLEGAL_ARGUMENT, "handleCommandInternalFailed");
            break;
        case CommandId::RESET:
            mImplContext->resetBuffer();
            break;
        default:
            break;
    }
    return ndk::ScopedAStatus::ok();
}

RetCode JamesDSPAIDL::releaseContext() {
    if (mContext) {
        int32_t ret = 0;
        uint32_t ret_size = sizeof(ret);
        int32_t status = mContext->handleCommand(EFFECT_CMD_RESET, 0, NULL, &ret_size, &ret);
        if (status < 0) {
            LOG(ERROR) << __func__ << ": JamesDSPAIDLContext::handleCommand RESET failed: " << status;
            return RetCode::ERROR_ILLEGAL_PARAMETER;
        }
        if (ret < 0) {
            LOG(ERROR) << __func__ << ": JamesDSPAIDLContext::handleCommand RESET failed (internal): " << ret;
            return RetCode::ERROR_ILLEGAL_PARAMETER;
        }
        mContext.reset();
    }
    return RetCode::SUCCESS;
}

// Processing method running in EffectWorker thread.
IEffect::Status JamesDSPAIDL::effectProcessImpl(float* in_, float* out_, int samples) {
    size_t frames = static_cast<size_t>(samples) / 2;
    audio_buffer_t in{frames, {in_}}, out{frames, {out_}};

#ifdef AIDL_DEBUG
    LOG(DEBUG) << __func__ << ": in " << in_ << ", out " << out_ << ", samples " << samples;
#endif
    int32_t ret = mContext->process(&in, &out);
#ifdef AIDL_DEBUG
    LOG(DEBUG) << __func__ << ": mContext->process: " << ret;
#endif

    switch(ret) {
        case 0:
            return {STATUS_OK, samples, samples};
        case -ENODATA:
            return {STATUS_NOT_ENOUGH_DATA, 0, 0};
        default:
            return {STATUS_INVALID_OPERATION, 0, 0};
    }
}

}  // namespace aidl::android::hardware::audio::effect
