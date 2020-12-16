////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                               //                                                                               //
//  Copyright ou © ou Copr.                                                      //  Copyright or © or Copr.                                                      //
//  Florian LAHAYE <florian@lahaye.me>(2020)                                     //  Florian LAHAYE <florian@lahaye.me>(2020)                                     //
//                                                                               //                                                                               //
//  Ce logiciel est un programme informatique permettant la gestion de tâches    //  This software is a computer program to manage parallel tasks.                //
//  parallèle.                                                                   //                                                                               //
//                                                                               //                                                                               //
//  Ce logiciel est régi par la licence CeCILL - C soumise au droit français et  //  This software is governed by the CeCILL - C license under French law and     //
//  respectant les principes de diffusion des logiciels libres.Vous pouvez       //  abiding by the rules of distribution of free software.You can  use,          //
//  utiliser, modifier et / ou redistribuer ce programme sous les conditions     //  modify and / or redistribute the software under the terms of the CeCILL - C  //
//  de la licence CeCILL - C telle que diffusée par le CEA, le CNRS et l'INRIA   //  license as circulated by CEA, CNRS and INRIA at the following URL            //
//  sur le site "http://www.cecill.info".                                        //  "http://www.cecill.info".                                                    //
//                                                                               //                                                                               //
//  En contrepartie de l'accessibilité au code source et des droits de copie,    //  As a counterpart to the access to the source code and rights to copy,        //
//  de modification et de redistribution accordés par cette licence, il n'est    //  modify and redistribute granted by the license, users are provided only      //
//  offert aux utilisateurs qu'une garantie limitée.  Pour les mêmes raisons,    //  with a limited warranty and the software's author,  the holder of the        //
//  seule une responsabilité restreinte pèse sur l'auteur du programme,  le      //  economic rights, and the successive licensors  have only  limited            //
//  titulaire des droits patrimoniaux et les concédants successifs.              //  liability.                                                                   //
//                                                                               //                                                                               //
//  A cet égard  l'attention de l'utilisateur est attirée sur les risques        //  In this respect, the user's attention is drawn to the risks associated       //
//  associés au chargement, à l'utilisation,  à la modification et/ou au         //  with loading, using, modifying and /or developing or reproducing the         //
//  développement et à la reproduction du logiciel par l'utilisateur étant       //  software by the user in light of its specific status of free software,       //
//  donné sa spécificité de logiciel libre, qui peut le rendre complexe à        //  that may mean  that it is complicated to manipulate, and that  also          //
//  manipuler et qui le réserve donc à des développeurs et des professionnels    //  therefore means  that it is reserved for developers and experienced          //
//  avertis possédant  des  connaissances  informatiques approfondies.Les        //  professionals having in - depth computer knowledge.Users are therefore       //
//  utilisateurs sont donc invités à charger  et  tester  l'adéquation  du       //  encouraged to load and test the software's suitability as regards their      //
//  logiciel à leurs besoins dans des conditions permettant d'assurer la         //  requirements in conditions enabling the security of their systems and /or    //
//  sécurité de leurs systèmes et ou de leurs données et, plus généralement,     //  data to be ensuredand, more generally, to use and operate it in the          //
//  à l'utiliser et l'exploiter dans les mêmes conditions de sécurité.           //  same conditions as regards security.                                         //
//                                                                               //                                                                               //
//  Le fait que vous puissiez accéder à cet en - tête signifie que vous avez     //  The fact that you are presently reading this means that you have had         //
//  pris connaissance de la licence CeCILL - C, et que vous en avez accepté les  //  knowledge of the CeCILL - C license and that you accept its terms.           //
//  termes.                                                                      //                                                                               //
//                                                                               //                                                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef H_FBAFBD9E_A57F_4F77_B3B7_1398920219AF
#define H_FBAFBD9E_A57F_4F77_B3B7_1398920219AF

#include <memory>
#include <mutex>
#include <stdexcept>

#include <plz/ns.hpp>

namespace PLZ_NS {
    class BadSynchronizedAccess : ::std::runtime_error
    {
    public:
        explicit BadSynchronizedAccess() : ::std::runtime_error("Bad access on a synchronized data")
        {}
    };

    template <typename T>
    class synchronized_accessor final
    {
    public:
        using value_type = T;

    private:
        value_type* m_borrow{ nullptr };
        ::std::unique_lock<::std::mutex> m_lck;

    public:
        synchronized_accessor() = default;
        synchronized_accessor(synchronized_accessor const&) = delete;
        synchronized_accessor(synchronized_accessor&&) noexcept = default;
        synchronized_accessor& operator=(synchronized_accessor const&) = delete;
        synchronized_accessor& operator=(synchronized_accessor&&) noexcept = default;
        ~synchronized_accessor() noexcept = default;

        synchronized_accessor(::std::unique_lock<::std::mutex> lck, value_type* borrow)
            : m_borrow{ borrow }, m_lck{ ::std::move(lck) }
        {}

        value_type& operator*()
        {
            if (!*this) {
                throw BadSynchronizedAccess();
            }
            else {
                return *m_borrow;
            }
        }

        value_type const& operator*() const
        {
            if (!*this) {
                throw BadSynchronizedAccess();
            }
            else {
                return *m_borrow;
            }
        }

        value_type* operator->()
        {
            if (!*this) {
                throw BadSynchronizedAccess();
            }
            else {
                return m_borrow;
            }
        }

        value_type const* operator->() const
        {
            if (!*this) {
                throw BadSynchronizedAccess();
            }
            else {
                return m_borrow;
            }
        }

        operator bool() const
        {
            return m_borrow && m_lck;
        }

        void unlock()
        {
            m_lck.unlock();
        }

        void reacquire()
        {
            m_lck.lock();
        }

        void try_reacquire()
        {
            m_lck.try_lock();
        }
    };

    template <typename T>
    class synchronized_state final
    {
    public:
        using value_type = T;

    private:
        value_type m_item;
        mutable ::std::mutex m_mut;

    public:
        synchronized_state() = default;
        synchronized_state(synchronized_state const&) = delete;
        synchronized_state(synchronized_state&&) noexcept = delete;
        synchronized_state& operator=(synchronized_state const&) = delete;
        synchronized_state& operator=(synchronized_state&&) noexcept = delete;
        ~synchronized_state() noexcept = default;

        synchronized_state(value_type value)
            : m_item{ ::std::move(value) }
        {}

#ifdef _MSC_VER
        _Acquires_lock_(m_mut)
#endif
        synchronized_accessor<value_type> acquire()
        {
            auto lck = ::std::unique_lock{ m_mut };
            return synchronized_accessor<value_type>(::std::move(lck), &m_item);
        }

#ifdef _MSC_VER
        _Acquires_lock_(m_mut)
#endif
        synchronized_accessor<value_type> try_acquire()
        {
            auto lck = ::std::unique_lock(m_mut, ::std::try_to_lock);
            return synchronized_accessor<value_type>(::std::move(lck), &m_item);
        }
    };

    template <typename T>
    class shared_object final
    {
    public:
        using value_type = T;
        static_assert(::std::is_same_v<value_type, typename synchronized_state<value_type>::value_type>);

    private:
        ::std::shared_ptr<synchronized_state<value_type>> m_state_ptr{ ::std::make_shared<synchronized_state<value_type>>() };

    public:
        shared_object() = default;
        shared_object(shared_object const&) = default;
        shared_object(shared_object&&) noexcept = default;
        shared_object& operator=(shared_object const&) = default;
        shared_object& operator=(shared_object&&) noexcept = default;
        ~shared_object() noexcept = default;

        explicit shared_object(value_type value)
            : m_state_ptr{ ::std::make_shared<synchronized_state<value_type>>(::std::move(value)) }
        {}

        synchronized_accessor<value_type> acquire() const
        {
            if (m_state_ptr) {
                return m_state_ptr->acquire();
            }
            else {
                return {};
            }
        }

        synchronized_accessor<value_type> try_acquire() const
        {
            if (m_state_ptr) {
                return m_state_ptr->try_acquire();
            }
            else {
                return {};
            }
        }
    };
}

#endif /* H_FBAFBD9E_A57F_4F77_B3B7_1398920219AF */

// vim: ts=4 sw=4 et eol:
