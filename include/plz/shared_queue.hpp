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

#ifndef H_3FCC58CE_7C01_4810_981B_FF01C51A07E6
#define H_3FCC58CE_7C01_4810_981B_FF01C51A07E6

#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>

#include <plz/ns.hpp>

namespace PLZ_NS {
    template <typename T>
    class synchronized_queue final
    {
    public:
        using value_type = T;
        using size_type = typename ::std::deque<value_type>::size_type;

    private:
        ::std::deque<value_type> m_queue;
        mutable ::std::mutex m_mut;
        mutable ::std::condition_variable m_push_cv;
        mutable ::std::condition_variable m_pop_cv;
        size_type m_max_size{ ::std::numeric_limits<size_type>::max() };

    public:
        synchronized_queue() = default;
        synchronized_queue(synchronized_queue const&) = delete;
        synchronized_queue(synchronized_queue&&) noexcept = delete;
        synchronized_queue& operator=(synchronized_queue const&) = delete;
        synchronized_queue& operator=(synchronized_queue&&) noexcept = delete;

        ~synchronized_queue() noexcept
        {
            // Call clear to ensure order of deletion
            clear();
        }

        size_type estimate_size() const
        {
            auto lck = ::std::lock_guard{ m_mut };
            return m_queue.size();
        }

        void unset_max_size()
        {
            auto lck = ::std::lock_guard{ m_mut };
            m_max_size = ::std::numeric_limits<size_type>::max();
            m_push_cv.notify_all();
        }

        void set_max_size(size_type max_size)
        {
            auto lck = ::std::lock_guard{ m_mut };
            if (max_size > m_max_size) {
                m_push_cv.notify_all();
            }
            m_max_size = max_size;
        }

        template <typename... Args>
        void emplace(Args&&... args)
        {
            auto lck = ::std::unique_lock{ m_mut };

            m_push_cv.wait(lck, [this]() {
                return m_queue.size() < m_max_size;
            });

            m_queue.emplace_back(::std::forward<Args>(args)...);
            m_pop_cv.notify_one();
        }

        void push(value_type const& item)
        {
            emplace(item);
        }

        void push(value_type&& item)
        {
            emplace(::std::move(item));
        }

        template <typename Rep, typename Period>
        ::std::optional<value_type> wait_and_pop(::std::chrono::duration<Rep, Period> const& rel_time)
        {
            auto lck = ::std::unique_lock{ m_mut };
            m_pop_cv.wait_for(lck, rel_time, [this]() {
                return !m_queue.empty();
            });

            auto popped_value = ::std::optional<value_type>{};
            if (!m_queue.empty()) {
                popped_value = ::std::move(m_queue.front());
                m_queue.pop_front();
                m_push_cv.notify_one();
            }

            return popped_value;
        }

        ::std::optional<value_type> try_pop()
        {
            auto popped_value = ::std::optional<value_type>{};
            if (auto lck = ::std::unique_lock(m_mut, ::std::try_to_lock)) {
                if (!m_queue.empty()) {
                    popped_value = ::std::move(m_queue.front());
                    m_queue.pop_front();
                    m_push_cv.notify_one();
                }
            }

            return popped_value;
        }

        template <typename Rep, typename Period>
        ::std::optional<value_type> wait_and_pop_last(::std::chrono::duration<Rep, Period> const& rel_time)
        {
            auto lck = ::std::unique_lock{ m_mut };
            m_pop_cv.wait_for(lck, rel_time, [this]() {
                return !m_queue.empty();
            });

            auto popped_value = ::std::optional<value_type>{};
            if (!m_queue.empty()) {
                popped_value = ::std::move(m_queue.back());

                // Loop over items to ensure order of deletion
                while (!m_queue.empty()) {
                    m_queue.pop_front();
                }

                m_push_cv.notify_all();
            }

            return popped_value;
        }

        ::std::optional<value_type> try_pop_last()
        {
            auto popped_value = ::std::optional<value_type>{};
            if (auto lck = ::std::unique_lock(m_mut, ::std::try_to_lock)) {
                if (!m_queue.empty()) {
                    popped_value = ::std::move(m_queue.back());

                    // Loop over items to ensure order of deletion
                    while (!m_queue.empty()) {
                        m_queue.pop_front();
                    }

                    m_push_cv.notify_all();
                }
            }

            return popped_value;
        }

        void clear() noexcept
        {
            auto lck = ::std::lock_guard{ m_mut };

            // Loop over items to ensure order of deletion
            while (!m_queue.empty()) {
                m_queue.pop_front();
            }

            m_push_cv.notify_all();
        }
    };

    template <typename T>
    class shared_queue final
    {
    public:
        using value_type = T;
        static_assert(::std::is_same_v<value_type, typename synchronized_queue<value_type>::value_type>);

        using size_type = typename synchronized_queue<value_type>::size_type;

    private:
        ::std::shared_ptr<synchronized_queue<value_type>> m_queue_ptr;

    public:
        shared_queue()
            : m_queue_ptr{ ::std::make_shared<synchronized_queue<value_type>>() }
        {}

        shared_queue(nullptr_t)
        {}

        operator bool() const
        {
            return static_cast<bool>(m_queue_ptr);
        }

        ::std::weak_ptr<synchronized_queue<value_type>> weak() const
        {
            return m_queue_ptr;
        }

        size_type estimate_size() const
        {
            if (m_queue_ptr) {
                return m_queue_ptr->estimate_size();
            }
            else {
                return 0;
            }
        }

        void unset_max_size() const
        {
            if (m_queue_ptr) {
                m_queue_ptr->unset_max_size();
            }
        }

        void set_max_size(size_type max_size) const
        {
            if (m_queue_ptr) {
                m_queue_ptr->set_max_size(max_size);
            }
        }

        template <typename... Args>
        void emplace(Args&&... args) const
        {
            if (m_queue_ptr) {
                m_queue_ptr->emplace(::std::forward<Args>(args)...);
            }
        }

        void push(value_type const& item) const
        {
            if (m_queue_ptr) {
                m_queue_ptr->push(item);
            }
        }

        void push(value_type&& item) const
        {
            if (m_queue_ptr) {
                m_queue_ptr->push(::std::move(item));
            }
        }

        template <typename Rep, typename Period>
        ::std::optional<value_type> wait_and_pop(::std::chrono::duration<Rep, Period> const& rel_time) const
        {
            if (m_queue_ptr) {
                return m_queue_ptr->wait_and_pop(rel_time);
            }
            else {
                return {};
            }
        }

        ::std::optional<value_type> wait_and_pop() const
        {
            using namespace ::std::chrono_literals;
            return wait_and_pop(10ms);
        }

        ::std::optional<value_type> try_pop() const
        {
            if (m_queue_ptr) {
                return m_queue_ptr->try_pop();
            }
            else {
                return {};
            }
        }

        template <typename Rep, typename Period>
        ::std::optional<value_type> wait_and_pop_last(::std::chrono::duration<Rep, Period> const& rel_time) const
        {
            if (m_queue_ptr) {
                return m_queue_ptr->wait_and_pop_last(rel_time);
            }
            else {
                return {};
            }
        }

        ::std::optional<value_type> wait_and_pop_last() const
        {
            using namespace ::std::chrono_literals;
            return wait_and_pop_last(10ms);
        }

        ::std::optional<value_type> try_pop_last() const
        {
            if (m_queue_ptr) {
                return m_queue_ptr->try_pop_last();
            }
            else {
                return {};
            }
        }

        void clear() const
        {
            if (m_queue_ptr) {
                m_queue_ptr->clear();
            }
        }
    };

    template <typename T>
    class fast_weak_queue final
    {
    public:
        using value_type = T;
        static_assert(::std::is_same_v<value_type, typename synchronized_queue<value_type>::value_type>);

        using size_type = typename synchronized_queue<value_type>::size_type;

    private:
        ::std::weak_ptr<synchronized_queue<value_type>> m_queue_ptr;

    public:
        fast_weak_queue() = default;
        fast_weak_queue(fast_weak_queue const&) = default;
        fast_weak_queue(fast_weak_queue&&) noexcept = default;
        fast_weak_queue& operator=(fast_weak_queue const&) = default;
        fast_weak_queue& operator=(fast_weak_queue&&) noexcept = default;
        ~fast_weak_queue() noexcept = default;

        fast_weak_queue(shared_queue<T> shared)
            : m_queue_ptr{ shared.weak() }
        {}

        operator bool() const
        {
            return static_cast<bool>(m_queue_ptr.lock());
        }

        size_type estimate_size() const
        {
            if (auto ptr = m_queue_ptr.lock()) {
                return ptr->estimate_size();
            }
            else {
                return 0;
            }
        }

        template <typename... Args>
        void emplace(Args&&... args) const
        {
            if (auto ptr = m_queue_ptr.lock()) {
                ptr->emplace(::std::forward<Args>(args)...);
            }
        }

        void push(value_type const& item) const
        {
            if (auto ptr = m_queue_ptr.lock()) {
                ptr->push(item);
            }
        }

        void push(value_type&& item) const
        {
            if (auto ptr = m_queue_ptr.lock()) {
                ptr->push(::std::move(item));
            }
        }
    };

    template <typename T>
    class weak_queue_adaptor_interface
    {
    private:
        virtual ::std::size_t do_estimate_size() const = 0;
        virtual void do_push(T const&) = 0;
        virtual void do_push(T&&) = 0;
        virtual ::std::unique_ptr<weak_queue_adaptor_interface<T>> do_clone() const = 0;
        virtual bool do_is_valid() const = 0;

    public:
        virtual ~weak_queue_adaptor_interface() = default;

        operator bool() const
        {
            return do_is_valid();
        }

        ::std::size_t estimate_size() const
        {
            return do_estimate_size();
        }

        void push(T const& t)
        {
            do_push(t);
        }

        void push(T&& t)
        {
            do_push(::std::move(t));
        }

        ::std::unique_ptr<weak_queue_adaptor_interface> clone() const
        {
            //TODO: type check
            return do_clone();
        }
    };

    template <typename T, typename U>
    class weak_queue_adaptor : public weak_queue_adaptor_interface<T>
    {
    private:
        virtual ::std::size_t do_estimate_size() const override
        {
            if (auto ptr = m_queue_ptr.lock()) {
                return ptr->estimate_size();
            }
            else {
                return 0;
            }
        }

        virtual void do_push(T const& item) override
        {
            if (auto ptr = m_queue_ptr.lock()) {
                ptr->push(item);
            }
        }

        virtual void do_push(T&& item) override
        {
            if (auto ptr = m_queue_ptr.lock()) {
                ptr->push(::std::move(item));
            }
        }

        virtual ::std::unique_ptr<weak_queue_adaptor_interface<T>> do_clone() const override
        {
            return ::std::make_unique<weak_queue_adaptor<T, U>>(m_queue_ptr);
        }

        virtual bool do_is_valid() const
        {
            return static_cast<bool>(m_queue_ptr.lock());
        }

    public:
        weak_queue_adaptor(::std::weak_ptr<synchronized_queue<U>> queue_ptr)
            : m_queue_ptr{ std::move(queue_ptr)}
        {}

        ::std::weak_ptr<synchronized_queue<U>> m_queue_ptr;
    };

    template <typename T>
    class weak_queue final
    {
    public:
        using value_type = T;
        static_assert(::std::is_same_v<value_type, typename synchronized_queue<value_type>::value_type>);

        using size_type = typename synchronized_queue<value_type>::size_type;

    private:
        ::std::unique_ptr<weak_queue_adaptor_interface<value_type>> m_uptr;

    public:
        weak_queue() = default;

        weak_queue(weak_queue const& other)
        {
            if (other.m_uptr) {
                m_uptr = other.m_uptr->clone();
            }
        }

        template <typename U>
        weak_queue(weak_queue<U> const& other)
        {
            if (other.m_uptr) {
                m_uptr = other.m_uptr->clone();
            }
        }

        weak_queue(weak_queue&&) noexcept = default;
        
        weak_queue& operator=(weak_queue const& rhs)
        {
            if (this != &rhs) {
                if (rhs.m_uptr) {
                    m_uptr = rhs.m_uptr->clone();
                }
                else {
                    m_uptr.reset();
                }
            }

            return *this;
        }

        template <typename U>
        weak_queue& operator=(weak_queue<U> const& rhs)
        {
            if (this != &rhs) {
                if (rhs.m_uptr) {
                    m_uptr = rhs->clone();
                }
                else {
                    m_uptr.reset();
                }
            }

            return *this;
        }

        weak_queue& operator=(weak_queue&&) noexcept = default;
        ~weak_queue() noexcept = default;

        template <typename U>
        weak_queue(shared_queue<U> shared)
            : m_uptr{ ::std::make_unique<weak_queue_adaptor<value_type, U>>(shared.weak()) }
        {}

        operator bool() const
        {
            return static_cast<bool>(m_uptr) && static_cast<bool>(*m_uptr);
        }

        size_type estimate_size() const
        {
            if (m_uptr) {
                return m_uptr->estimate_size();
            }
            else {
                return 0;
            }
        }

        void push(value_type const& item) const
        {
            if (m_uptr) {
                m_uptr->push(item);
            }
        }

        void push(value_type&& item) const
        {
            if (m_uptr) {
                m_uptr->push(::std::move(item));
            }
        }
    };
}

#endif /* H_3FCC58CE_7C01_4810_981B_FF01C51A07E6 */

// vim: ts=4 sw=4 et eol:
