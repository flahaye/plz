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

#ifndef H_862271B1_8B7C_482E_B186_2145AEB7FD72
#define H_862271B1_8B7C_482E_B186_2145AEB7FD72

#include <atomic>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <type_traits>

#include <plz/io.hpp>
#include <plz/ns.hpp>

namespace PLZ_NS {
    enum class error_code : int
    {
        CONTINUE = 0, KILL_UNIT, TERMINATE
    };

    class ignore_error
    {
    protected:
        error_code on_error(::std::exception_ptr)
        {
            return error_code::CONTINUE;
        }
    };

    class print_and_rethrow_on_error
    {
    protected:
        error_code on_error(::std::exception_ptr eptr)
        {
            try {
                ::std::rethrow_exception(eptr);
            }
            catch (::std::exception const& e) {
                ::std::cerr << e.what() << ::std::endl;
                throw;
            }
            catch (...) {
                throw;
            }
            return error_code::TERMINATE;
        }
    };

    class terminate_on_error
    {
    protected:
        error_code on_error(::std::exception_ptr)
        {
            return error_code::TERMINATE;
        }
    };

    class kill_unit_on_error
    {
    protected:
        error_code on_error(::std::exception_ptr)
        {
            return error_code::KILL_UNIT;
        }
    };

    template <typename Input, typename Output, typename Shared>
    struct unit_data
    {
        typename io::traits<Input>::inputs_type inputs;
        typename io::traits<Output>::outputs_type outputs;
        typename io::traits<Shared>::shareds_type shareds;
    };

    template <
        typename Input = io::list<>, typename Output = io::list<>, typename Shared = io::list<>
    >
    class unit_spec
    {
    public:
        using input_spec = Input;
        using output_spec = Output;
        using shared_spec = Shared;

        using unit_data_t = unit_data<Input, Output, Shared>;

    private:
        static_assert(io::is_input_v<input_spec>, "The first template argument of " PLZ_NS_STR "::unit_spec should be a specification of input data");
        static_assert(io::is_output_v<output_spec>, "The second template argument of " PLZ_NS_STR "::unit_spec should be a specification of output data");
        static_assert(io::is_shared_v<shared_spec>, "The third template argument of " PLZ_NS_STR "::unit_spec should be a specification of shared data");
    
        unit_data_t m_data;

    protected:
        unit_spec() = default;

        unit_spec(unit_spec const&) = default;
        unit_spec(unit_spec&&) noexcept = default;
        unit_spec& operator=(unit_spec const&) = default;
        unit_spec& operator=(unit_spec&&) noexcept = default;
        ~unit_spec() noexcept = default;

        template <typename io::traits<input_spec>::index_type Idx>
        typename io::traits<input_spec>::template input_type<Idx> const& input_queue() const
        {
            using ::std::get;
            return get<io::traits<input_spec>::inputs_idx<Idx>>(m_data.inputs);
        }

        template <typename io::traits<output_spec>::index_type Idx>
        typename io::traits<output_spec>::template output_type<Idx> const& output_queue() const
        {
            using ::std::get;
            return get<io::traits<output_spec>::outputs_idx<Idx>>(m_data.outputs);
        }

        template <typename io::traits<shared_spec>::index_type Idx>
        typename io::traits<shared_spec>::template shared_type<Idx> const& shared_object() const
        {
            using ::std::get;
            return get<io::traits<shared_spec>::shareds_idx<Idx>>(m_data.shareds);
        }

    public:
        bool has_data() const
        {
            return ::std::apply([](auto&... shared_queues) {
                auto not_empty = [](auto& shared_queue) {
                    return shared_queue.estimate_size() > 0U;
                };
                return (not_empty(shared_queues) || ...);
            }, m_data.inputs);
        }

        void set_unit_data(unit_data_t const& data)
        {
            m_data = data;
        }

        void set_unit_data(unit_data_t&& data)
        {
            m_data = ::std::move(data);
        }
    };

    struct start_worker_t {};
    inline constexpr start_worker_t start_worker;

    template<
        typename UnitSpec, typename ErrorPolicy = print_and_rethrow_on_error
    >
    class worker final : public ErrorPolicy
    {
    private:
        using unit_spec_type = UnitSpec;

        using input_spec = typename unit_spec_type::input_spec;
        using output_spec = typename unit_spec_type::output_spec;
        using shared_spec = typename unit_spec_type::shared_spec;

        using unit_data_t = typename unit_spec_type::unit_data_t;

        unit_data_t m_unit_data;

        ::std::atomic<bool> m_done{ false };
        ::std::atomic<bool> m_kill{ false };
        ::std::thread m_worker;

    public:
        worker() = default;
        worker(worker const&) = delete;
        worker(worker&&) noexcept = default;
        worker& operator=(worker const&) = delete;
        worker& operator=(worker&&) noexcept = default;
        ~worker() noexcept
        {
            stop_and_join_unit();
        }

        template <typename... Args>
        worker(start_worker_t, Args&&... args)
        {
            restart_unit(::std::forward<Args>(args)...);
        }

        template <typename... Args>
        void restart_unit(Args&&... args)
        {
            stop_and_join_unit();

            m_done.store(false, ::std::memory_order_release);
            m_kill.store(false, ::std::memory_order_release);
            m_worker = ::std::thread([this, unit_data = m_unit_data](::std::decay_t<Args>... fn_args) mutable {
                auto my_unit = unit_spec_type(::std::move(fn_args)...);
                my_unit.set_unit_data(::std::move(unit_data));

                auto done = false;
                while (!done) {
                    auto err = error_code::CONTINUE;

                    try {
                        if constexpr (::std::is_same_v<decltype(my_unit()), void>) {
                            my_unit();
                        }
                        else {
                            err = my_unit();
                        }
                    }
                    catch (...) {
                        err = ErrorPolicy::on_error(::std::current_exception());
                    }

                    if (err == error_code::KILL_UNIT) {
                        return;
                    }
                    else if (err == error_code::TERMINATE) {
                        ::std::terminate();
                    }

                    done = m_kill.load(::std::memory_order_acquire)
                        || m_done.load(::std::memory_order_acquire) && !my_unit.has_data();
                }
            }, ::std::forward<Args>(args)...);
        }

        bool unit_is_running() const
        {
            return m_worker.joinable();
        }

        void join_unit()
        {
            if (m_worker.joinable()) {
                m_worker.join();
            }
        }

        void send_stop_unit_request()
        {
            m_done.store(true, ::std::memory_order_release);
        }

        void send_kill_unit_request()
        {
            m_kill.store(true, ::std::memory_order_release);
        }

        void stop_and_join_unit()
        {
            if (m_worker.joinable()) {
                send_stop_unit_request();
                m_worker.join();
            }
        }

        void kill_and_join_unit()
        {
            if (m_worker.joinable()) {
                send_kill_unit_request();
                m_worker.join();
            }
        }

        template <typename io::traits<input_spec>::index_type Idx>
        typename io::traits<input_spec>::template input_type<Idx> const& input_queue() const
        {
            using ::std::get; 
            return get<io::traits<input_spec>::inputs_idx<Idx>>(m_unit_data.inputs);
        }

        template <typename io::traits<output_spec>::index_type Idx>
        typename io::traits<output_spec>::template output_type<Idx> const& output_queue() const
        {
            using ::std::get;
            return get<io::traits<output_spec>::outputs_idx<Idx>>(m_unit_data.outputs);
        }

        template <typename io::traits<shared_spec>::index_type Idx>
        typename io::traits<shared_spec>::template shared_type<Idx> const& shared_object() const
        {
            using ::std::get;
            return get<io::traits<shared_spec>::shareds_idx<Idx>>(m_unit_data.shareds);
        }

        template <typename io::traits<output_spec>::index_type Idx>
        void assign_output_queue(typename io::traits<output_spec>::template output_type<Idx> queue)
        {
            using ::std::get;
            get<io::traits<output_spec>::outputs_idx<Idx>>(m_unit_data.outputs) = ::std::move(queue);
        }

        template <typename io::traits<shared_spec>::index_type Idx>
        void assign_shared_object(typename io::traits<shared_spec>::template shared_type<Idx> object)
        {
            using ::std::get;
            get<io::traits<shared_spec>::shareds_idx<Idx>>(m_unit_data.shareds) = ::std::move(object);
        }
    };
}

#endif /* H_862271B1_8B7C_482E_B186_2145AEB7FD72 */

// vim: ts=4 sw=4 et eol:
