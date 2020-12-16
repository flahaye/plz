////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                               //                                                                               //
//  Copyright ou � ou Copr.                                                      //  Copyright or � or Copr.                                                      //
//  Florian LAHAYE <florian@lahaye.me>(2020)                                     //  Florian LAHAYE <florian@lahaye.me>(2020)                                     //
//                                                                               //                                                                               //
//  Ce logiciel est un programme informatique permettant la gestion de t�ches    //  This software is a computer program to manage parallel tasks.                //
//  parall�le.                                                                   //                                                                               //
//                                                                               //                                                                               //
//  Ce logiciel est r�gi par la licence CeCILL - C soumise au droit fran�ais et  //  This software is governed by the CeCILL - C license under French law and     //
//  respectant les principes de diffusion des logiciels libres.Vous pouvez       //  abiding by the rules of distribution of free software.You can  use,          //
//  utiliser, modifier et / ou redistribuer ce programme sous les conditions     //  modify and / or redistribute the software under the terms of the CeCILL - C  //
//  de la licence CeCILL - C telle que diffus�e par le CEA, le CNRS et l'INRIA   //  license as circulated by CEA, CNRS and INRIA at the following URL            //
//  sur le site "http://www.cecill.info".                                        //  "http://www.cecill.info".                                                    //
//                                                                               //                                                                               //
//  En contrepartie de l'accessibilit� au code source et des droits de copie,    //  As a counterpart to the access to the source code and rights to copy,        //
//  de modification et de redistribution accord�s par cette licence, il n'est    //  modify and redistribute granted by the license, users are provided only      //
//  offert aux utilisateurs qu'une garantie limit�e.  Pour les m�mes raisons,    //  with a limited warranty and the software's author,  the holder of the        //
//  seule une responsabilit� restreinte p�se sur l'auteur du programme,  le      //  economic rights, and the successive licensors  have only  limited            //
//  titulaire des droits patrimoniaux et les conc�dants successifs.              //  liability.                                                                   //
//                                                                               //                                                                               //
//  A cet �gard  l'attention de l'utilisateur est attir�e sur les risques        //  In this respect, the user's attention is drawn to the risks associated       //
//  associ�s au chargement, � l'utilisation,  � la modification et/ou au         //  with loading, using, modifying and /or developing or reproducing the         //
//  d�veloppement et � la reproduction du logiciel par l'utilisateur �tant       //  software by the user in light of its specific status of free software,       //
//  donn� sa sp�cificit� de logiciel libre, qui peut le rendre complexe �        //  that may mean  that it is complicated to manipulate, and that  also          //
//  manipuler et qui le r�serve donc � des d�veloppeurs et des professionnels    //  therefore means  that it is reserved for developers and experienced          //
//  avertis poss�dant  des  connaissances  informatiques approfondies.Les        //  professionals having in - depth computer knowledge.Users are therefore       //
//  utilisateurs sont donc invit�s � charger  et  tester  l'ad�quation  du       //  encouraged to load and test the software's suitability as regards their      //
//  logiciel � leurs besoins dans des conditions permettant d'assurer la         //  requirements in conditions enabling the security of their systems and /or    //
//  s�curit� de leurs syst�mes et ou de leurs donn�es et, plus g�n�ralement,     //  data to be ensuredand, more generally, to use and operate it in the          //
//  � l'utiliser et l'exploiter dans les m�mes conditions de s�curit�.           //  same conditions as regards security.                                         //
//                                                                               //                                                                               //
//  Le fait que vous puissiez acc�der � cet en - t�te signifie que vous avez     //  The fact that you are presently reading this means that you have had         //
//  pris connaissance de la licence CeCILL - C, et que vous en avez accept� les  //  knowledge of the CeCILL - C license and that you accept its terms.           //
//  termes.                                                                      //                                                                               //
//                                                                               //                                                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef H_ECDDBB8D_2BC6_46CF_BCAD_F21340D63ED4
#define H_ECDDBB8D_2BC6_46CF_BCAD_F21340D63ED4

#include <tuple>
#include <type_traits>

#include <plz/ns.hpp>
#include <plz/shared_object.hpp>
#include <plz/shared_queue.hpp>

namespace PLZ_NS::io {
    template <typename...>
    struct list {};

    template <::std::size_t Idx, typename... Args>
    struct _get
    {};

    template <::std::size_t Idx, typename Arg, typename... Args>
    struct _get<Idx, Arg, Args...>
    {
        using type = typename _get<Idx - 1, Args...>::type;
    };

    template <typename Arg, typename... Args>
    struct _get<0, Arg, Args...>
    {
        using type = Arg;
    };

    template <::std::size_t Idx>
    struct _get<Idx>
    {};

    struct default_traits
    {
        using index_type = ::std::size_t;

        template <default_traits::index_type Idx>
        static inline constexpr ::std::size_t inputs_idx = Idx;

        template <default_traits::index_type Idx>
        static inline constexpr ::std::size_t outputs_idx = Idx;

        template <default_traits::index_type Idx>
        static inline constexpr ::std::size_t shareds_idx = Idx;
    };

    template <typename>
    struct traits {};

    template <typename... Args>
    struct traits<list<Args...>>
        : default_traits
    {
        template <default_traits::index_type Idx>
        using type_at = typename _get<Idx, Args...>::type;

        template <default_traits::index_type Idx>
        using input_type = ::PLZ_NS::shared_queue<type_at<Idx>>;

        using inputs_type = ::std::tuple<::PLZ_NS::shared_queue<Args>...>;

        template <default_traits::index_type Idx>
        using output_type = ::PLZ_NS::weak_queue<type_at<Idx>>;

        using outputs_type = ::std::tuple<::PLZ_NS::weak_queue<Args>...>;

        template <default_traits::index_type Idx>
        using shared_type = ::PLZ_NS::shared_object<type_at<Idx>>;

        using shareds_type = ::std::tuple<::PLZ_NS::shared_object<Args>...>;
    };

    template <typename T, typename traits<T>::index_type Idx>
    using type_at_t = typename traits<T>::template type_at<Idx>;

    template <typename T, typename = ::std::void_t<>>
    struct is_input : ::std::false_type
    {};

    template <typename T>
    struct is_input<T, ::std::void_t<typename traits<T>::input_type>> : ::std::true_type
    {};

    template <typename T>
    inline constexpr bool is_input_v = is_input<T>::value;

    template <typename T, typename = ::std::void_t<>>
    struct is_output : ::std::false_type
    {};

    template <typename T>
    struct is_output<T, ::std::void_t<typename traits<T>::output_type>> : ::std::true_type
    {};

    template <typename T>
    inline constexpr bool is_output_v = is_output<T>::value;

    template <typename T, typename = ::std::void_t<>>
    struct is_shared : ::std::false_type
    {};

    template <typename T>
    struct is_shared<T, ::std::void_t<typename traits<T>::shared_type>> : ::std::true_type
    {};

    template <typename T>
    inline constexpr bool is_shared_v = is_shared<T>::value;
}

#endif /* H_ECDDBB8D_2BC6_46CF_BCAD_F21340D63ED4 */

// vim: ts=4 sw=4 et eol:
