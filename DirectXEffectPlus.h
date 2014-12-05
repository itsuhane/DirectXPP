
/**
             Copyright itsuhane@gmail.com, 2014.
  Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
**/

#pragma once

#include "DirectXPlus.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler")

#include <d3dx11effect.h>
#include <SimpleMath.h>

/* class C { */
#define INJECT_EFFECT_COMOBJ_CONCEPT(C,I)                                       \
    private:                                                                    \
        I *m_##C = nullptr;                                                     \
        void reclaim()                                                          \
        {                                                                       \
            if(nullptr != m_##C) {                                              \
                DEBUG_REF(m_##C->AddRef());                                     \
            }                                                                   \
        }                                                                       \
                                                                                \
    protected:                                                                  \
        bool m_strict = false;                                                  \
                                                                                \
    public:                                                                     \
        void release() {                                                        \
            if(nullptr != m_##C) {                                              \
                DEBUG_REF(m_##C->Release());                                    \
                m_##C = nullptr;                                                \
            }                                                                   \
        }                                                                       \
                                                                                \
        explicit C(I *p##C, bool check_with_exception)                          \
            : m_##C(p##C), m_strict(check_with_exception)                       \
        {                                                                       \
            reclaim();                                                          \
        }                                                                       \
                                                                                \
        C(const C &c)                                                           \
        {                                                                       \
            release();                                                          \
            m_##C = c.m_##C;                                                    \
            reclaim();                                                          \
        }                                                                       \
                                                                                \
        C(C &&c)                                                                \
        {                                                                       \
            release();                                                          \
            m_##C = c.m_##C;                                                    \
            c.m_##C = nullptr;                                                  \
        }                                                                       \
                                                                                \
        ~C()                                                                    \
        {                                                                       \
            release();                                                          \
        }                                                                       \
                                                                                \
        C &operator= (const C &c)                                               \
        {                                                                       \
            if(this != &c) {                                                    \
                release();                                                      \
                m_##C = c.m_##C;                                                \
                reclaim();                                                      \
            }                                                                   \
            return *this;                                                       \
        }                                                                       \
                                                                                \
        C &operator= (C &&c)                                                    \
        {                                                                       \
            if(this != &c) {                                                    \
                release();                                                      \
                m_##C = c.m_##C;                                                \
                c.m_##C = nullptr;                                              \
            }                                                                   \
            return *this;                                                       \
        }                                                                       \
                                                                                \
        bool is_valid() const                                                   \
        {                                                                       \
            if(nullptr == m_##C) {                                              \
                return false;                                                   \
            } else {                                                            \
                return m_##C->IsValid();                                        \
            }                                                                   \
        }                                                                       \
                                                                                \
        I *winapi() const                                                       \
        {                                                                       \
            return m_##C;                                                       \
        }                                                                       \
                                                                                \
        template<class T> T as();
/* } */

namespace dx {

    namespace effect11 {

        class effect_runtime_error : public runtime_error {
        public:
            effect_runtime_error(HRESULT hr, const std::string &error)
                : runtime_error(hr), m_error(error)
            {}

            const std::string &error() const
            {
                return m_error;
            }

        private:
            std::string m_error;
        };

        class scalar {
            INJECT_EFFECT_COMOBJ_CONCEPT(scalar, ID3DX11EffectScalarVariable)
        public:
            scalar() {}
            scalar &operator=(float v)
            {
                throw_if_failed(m_scalar->SetFloat(v));
                return *this;
            }
        };

        class vector {
            INJECT_EFFECT_COMOBJ_CONCEPT(vector, ID3DX11EffectVectorVariable)
        public:
            vector() {}
            vector &operator=(const DirectX::SimpleMath::Vector4 &v)
            {
                throw_if_failed(m_vector->SetFloatVector((const float*)&v));
                return *this;
            }
        };

        class matrix {
            INJECT_EFFECT_COMOBJ_CONCEPT(matrix, ID3DX11EffectMatrixVariable)
        public:
            matrix() {}
            matrix &operator=(const DirectX::SimpleMath::Matrix &m)
            {
                throw_if_failed(m_matrix->SetMatrix((const float*)&m));
                return *this;
            }
        };

        class string {
            INJECT_EFFECT_COMOBJ_CONCEPT(string, ID3DX11EffectStringVariable)
        public:
            string() {}
        };

        class variable {
            INJECT_EFFECT_COMOBJ_CONCEPT(variable, ID3DX11EffectVariable)

        public:
            variable() {}

            template<>
            scalar as<scalar>()
            {
                ID3DX11EffectScalarVariable *s = m_variable->AsScalar();
                if(!s->IsValid()) {
                    s->Release();
                    if(m_strict) {
                        throw effect_runtime_error(S_OK, "Failed to cast as scalar");
                    } else {
                        return scalar();
                    }
                }
                return make_comobj<scalar>(s, m_strict);
            }

            template<>
            vector as<vector>()
            {
                ID3DX11EffectVectorVariable *vec = m_variable->AsVector();
                if(!vec->IsValid()) {
                    vec->Release();
                    if(m_strict) {
                        throw effect_runtime_error(S_OK, "Failed to cast as vector");
                    } else {
                        return vector();
                    }
                }
                return make_comobj<vector>(vec, m_strict);
            }

            template<>
            matrix as<matrix>()
            {
                ID3DX11EffectMatrixVariable *mat = m_variable->AsMatrix();
                if(!mat->IsValid()) {
                    mat->Release();
                    if(m_strict) {
                        throw effect_runtime_error(S_OK, "Failed to cast as matrix");
                    } else {
                        return matrix();
                    }
                }
                return make_comobj<matrix>(mat, m_strict);
            }

            template<>
            string as<string>()
            {
                ID3DX11EffectStringVariable *s = m_variable->AsString();
                if(!s->IsValid()) {
                    s->Release();
                    if(m_strict) {
                        throw effect_runtime_error(S_OK, "Failed to cast as string");
                    } else {
                        return string();
                    }
                }
                return make_comobj<string>(s, m_strict);
            }

        };

        class pass {
            INJECT_EFFECT_COMOBJ_CONCEPT(pass, ID3DX11EffectPass)

        public:
            pass() {}

            void apply(const d3d11::devicecontext &context)
            {
                m_pass->Apply(0, context.winapi());
            }

            d3d11::inputlayout create_inputlayout(const std::vector<D3D11_INPUT_ELEMENT_DESC> &layout, const d3d11::device &device)
            {
                ID3D11InputLayout *pLayout = nullptr;
                D3DX11_PASS_DESC passDesc;
                throw_if_failed(m_pass->GetDesc(&passDesc));
                throw_if_failed(device.winapi()->CreateInputLayout(layout.data(), (UINT)layout.size(), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &pLayout));
                return make_comobj<d3d11::inputlayout>(pLayout);
            }

        };

        class technique {
            INJECT_EFFECT_COMOBJ_CONCEPT(technique, ID3DX11EffectTechnique)

        public:
            technique() {}

            dx::effect11::pass pass(const std::string &name)
            {
                ID3DX11EffectPass *pPass = m_technique->GetPassByName(name.c_str());
                if(!pPass->IsValid()) {
                    pPass->Release();
                    if(m_strict) {
                        throw effect_runtime_error(S_OK, "No such pass.");
                    } else {
                        return dx::effect11::pass();
                    }
                }
                return make_comobj<dx::effect11::pass>(pPass, m_strict);
            }

            dx::effect11::pass pass(uint32_t id)
            {
                ID3DX11EffectPass *pPass = m_technique->GetPassByIndex(id);
                if(!pPass->IsValid()) {
                    pPass->Release();
                    if(m_strict) {
                        throw effect_runtime_error(S_OK, "No such pass.");
                    } else {
                        return dx::effect11::pass();
                    }
                }
                return make_comobj<dx::effect11::pass>(pPass, m_strict);
            }

        };

        class effect {

            INJECT_EFFECT_COMOBJ_CONCEPT(effect, ID3DX11Effect)

            struct BlobDeleter {
                void operator()(ID3DBlob *p) const
                {
                    if(nullptr != p) {
                        p->Release();
                    }
                }
            };

        public:

            effect() {}

            static effect createFromSource(const std::string &source, const d3d11::device &device, bool check_with_exception = false)
            {
                UINT compileFlags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;
            #ifdef _DEBUG
                compileFlags |= D3DCOMPILE_DEBUG;
            #endif
                ID3DBlob *pCode = nullptr;
                ID3DBlob *pError = nullptr;
                HRESULT hr = D3DCompile(source.data(), source.size(), nullptr, nullptr, nullptr, nullptr, "fx_5_0", compileFlags, 0, &pCode, &pError);
                std::unique_ptr<ID3DBlob, BlobDeleter> code(pCode);
                std::unique_ptr<ID3DBlob, BlobDeleter> error(pError);
                if(FAILED(hr)) {
                    throw effect_runtime_error(hr, (char*)error->GetBufferPointer());
                }

                ID3DX11Effect *pEffect = nullptr;
                throw_if_failed(D3DX11CreateEffectFromMemory(code->GetBufferPointer(), code->GetBufferSize(), 0, device.winapi(), &pEffect));
                return make_comobj<effect>(pEffect, check_with_exception);
            }

            dx::effect11::technique technique(const std::string &name)
            {
                ID3DX11EffectTechnique *pTech = m_effect->GetTechniqueByName(name.c_str());
                if(!pTech->IsValid()) {
                    pTech->Release();
                    if(m_strict) {
                        throw effect_runtime_error(S_OK, "No such technique.");
                    } else {
                        return dx::effect11::technique();
                    }
                }
                return make_comobj<dx::effect11::technique>(pTech, m_strict);
            }

            dx::effect11::technique technique(uint32_t id)
            {
                ID3DX11EffectTechnique *pTech = m_effect->GetTechniqueByIndex(id);
                if(!pTech->IsValid()) {
                    pTech->Release();
                    if(m_strict) {
                        throw effect_runtime_error(S_OK, "No such technique.");
                    } else {
                        return dx::effect11::technique();
                    }
                }
                return make_comobj<dx::effect11::technique>(pTech, m_strict);
            }

            variable variable_by_index(uint32_t id)
            {
                ID3DX11EffectVariable *var = m_effect->GetVariableByIndex(id);
                if(!var->IsValid()) {
                    var->Release();
                    if(m_strict) {
                        throw effect_runtime_error(S_OK, "No such variable");
                    } else {
                        return variable();
                    }
                }
                return make_comobj<variable>(var, m_strict);
            }

            variable variable_by_name(const std::string& name)
            {
                ID3DX11EffectVariable *var = m_effect->GetVariableByName(name.c_str());
                if(!var->IsValid()) {
                    var->Release();
                    if(m_strict) {
                        throw effect_runtime_error(S_OK, "No such variable");
                    } else {
                        return variable();
                    }
                }
                return make_comobj<variable>(var, m_strict);
            }

            variable variable_by_semantic(const std::string& semantic)
            {
                ID3DX11EffectVariable *var = m_effect->GetVariableBySemantic(semantic.c_str());
                if(!var->IsValid()) {
                    var->Release();
                    if(m_strict) {
                        throw effect_runtime_error(S_OK, "No such variable");
                    } else {
                        return variable();
                    }
                }
                return make_comobj<variable>(var, m_strict);
            }

        };

    } /* End of namespace effect11 */

} /* End of namespace dx */

#undef INJECT_EFFECT_COMOBJ_CONCEPT
