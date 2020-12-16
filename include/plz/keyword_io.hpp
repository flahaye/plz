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

#ifndef H_DA03DE8A_3A83_4660_8E36_4574A31DE067
#define H_DA03DE8A_3A83_4660_8E36_4574A31DE067

#include <tuple>
#include <type_traits>

#include <plz/io.hpp>
#include <plz/ns.hpp>
#include <plz/shared_object.hpp>
#include <plz/shared_queue.hpp>

namespace PLZ_NS::io::keyword {
    template <typename...>
    struct map {};

    template<auto Key, typename ValueType>
    struct item {};

    template <::std::size_t Offset, auto Key, typename... Args>
    struct _get
    {};

    template <::std::size_t Offset, auto Key, auto Key2, typename Value, typename... Args>
    struct _get<Offset, Key, item<Key2, Value>, Args...>
    {
        using type = typename _get<Offset + 1, Key, Args...>::type;
        static inline constexpr ::std::size_t idx = _get<Offset + 1, Key, Args...>::idx;
    };

    template <::std::size_t Offset, auto Key, typename Value, typename... Args>
    struct _get<Offset, Key, item<Key, Value>, Args...>
    {
        using type = Value;
        static inline constexpr ::std::size_t idx = Offset;
    };

    template <::std::size_t Offset, auto Key>
    struct _get<Offset, Key>
    {};

    template<auto Key, typename... Args>
    using get = _get<0, Key, Args...>;

    template <auto... Keys>
    struct _unique
    {};
   
    template <auto Key1, auto Key2, auto... Keys>
    struct _unique<Key1, Key2, Keys...> : std::conditional_t<
        Key1 == Key2,
        ::std::false_type,
        ::std::conjunction<
            _unique<Key1, Keys...>,
            _unique<Key2, Keys...>
        >
    >
    {};

    template <auto Key>
    struct _unique<Key> : ::std::true_type
    {};

    template <>
    struct _unique<> : ::std::true_type
    {};

    template <typename KeyType, KeyType... Keys, typename... Args>
    struct ::PLZ_NS::io::traits<map<item<Keys, Args>...>>
    {
        static_assert(keyword::_unique<Keys...>::value, "Duplicate keys found");

        using index_type = KeyType;

        template <index_type Key>
        using type_at = typename keyword::get<Key, keyword::item<Keys, Args>...>::type;

        template <index_type Key>
        using input_type = ::PLZ_NS::shared_queue<type_at<Key>>;

        using inputs_type = ::std::tuple<::PLZ_NS::shared_queue<Args>...>;

        template <index_type Key>
        static inline constexpr ::std::size_t inputs_idx = keyword::get<Key, keyword::item<Keys, Args>...>::idx;

        template <index_type Key>
        using output_type = ::PLZ_NS::weak_queue<type_at<Key>>;

        using outputs_type = ::std::tuple<::PLZ_NS::weak_queue<Args>...>;

        template <index_type Key>
        static inline constexpr ::std::size_t outputs_idx = keyword::get<Key, keyword::item<Keys, Args>...>::idx;

        template <index_type Key>
        using shared_type = ::PLZ_NS::shared_object<type_at<Key>>;

        using shareds_type = ::std::tuple<::PLZ_NS::shared_object<Args>...>;

        template <index_type Key>
        static inline constexpr ::std::size_t shareds_idx = keyword::get<Key, keyword::item<Keys, Args>...>::idx;
    };
}

#endif /* H_DA03DE8A_3A83_4660_8E36_4574A31DE067 */

// vim: ts=4 sw=4 et eol:
