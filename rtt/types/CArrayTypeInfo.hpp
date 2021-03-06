#ifndef ORO_TEMPLATE_CARRAY_INFO_HPP
#define ORO_TEMPLATE_CARRAY_INFO_HPP

#include "TemplateTypeInfo.hpp"
#include "../internal/ArrayPartDataSource.hpp"
#include "type_discovery.hpp"
#include <boost/lexical_cast.hpp>
#include "../internal/carray.hpp"

namespace RTT
{
    namespace types
    {
        /**
         * Template for data types that are C-style arrays.
         *
         * C-style arrays are represented by the carray<T> wrapper
         * in the RTT type system.
         * @param T A carray<U> wrapper, where U is a C data type.
         */
        template<typename T, bool has_ostream = false>
        class CArrayTypeInfo: public TemplateTypeInfo<T, has_ostream>
        {
        public:
            CArrayTypeInfo(std::string name) :
                TemplateTypeInfo<T, has_ostream> (name)
            {
            }

            virtual std::vector<std::string> getMemberNames() const {
                // only discover the parts of this struct:
                std::vector<std::string> result;
                result.push_back("size");
                result.push_back("capacity");
                return result;
            }

            virtual base::DataSourceBase::shared_ptr getMember(base::DataSourceBase::shared_ptr item, const std::string& name) const {
                using namespace internal;
                typename AssignableDataSource<T>::shared_ptr data = boost::dynamic_pointer_cast< AssignableDataSource<T> >( item );
                if ( !data ) {
                    return base::DataSourceBase::shared_ptr();
                }

                // size and capacity can not change during program execution:
                if (name == "size" || name == "capacity") {
                    return new ValueDataSource<unsigned int>( data->set().count() );
                }

                // contents of indx can change during program execution:
                try {
                    unsigned int indx = boost::lexical_cast<unsigned int>(name);
                    // @todo could also return a direct reference to item indx using another DS type that respects updated().
                    return new ArrayPartDataSource<typename T::value_type>( *data->set().address(), new ConstantDataSource<unsigned int>(indx), item );
                } catch(...) {}
                log(Error) << "CArrayTypeInfo: No such part (or invalid index): " << name << endlog();
                return base::DataSourceBase::shared_ptr();
            }

            virtual base::DataSourceBase::shared_ptr getMember(base::DataSourceBase::shared_ptr item,
                                                             base::DataSourceBase::shared_ptr id) const {
                using namespace internal;
                typename AssignableDataSource<T>::shared_ptr data = boost::dynamic_pointer_cast< AssignableDataSource<T> >( item );
                if ( !data ) {
                    return base::DataSourceBase::shared_ptr();
                }

                // discover if user gave us a part name or index:
                typename DataSource<unsigned int>::shared_ptr id_indx = DataSource<unsigned int>::narrow( id.get() );
                typename DataSource<string>::shared_ptr id_name = DataSource<string>::narrow( id.get() );
                if ( id_name ) {
                    // size and capacity can not change during program execution:
                    if (id_name->get() == "size" || id_name->get() == "capacity") {
                        return new ValueDataSource<unsigned int>( data->set().count() );
                    }
                }

                if ( id_indx ) {
                    return new ArrayPartDataSource<typename T::value_type>( *data->set().address(), id_indx, item );
                }
                log(Error) << "CArrayTypeInfo: No such part (or invalid index): " << id_name->get() << id_indx->get() << endlog();
                return base::DataSourceBase::shared_ptr();
            }
        };
    }
}

#endif
