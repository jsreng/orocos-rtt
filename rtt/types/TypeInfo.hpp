/***************************************************************************
  tag: Peter Soetens  Mon Jan 19 14:11:26 CET 2004  TypeInfo.hpp

                        TypeInfo.hpp -  description
                           -------------------
    begin                : Mon January 19 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public                   *
 *   License as published by the Free Software Foundation;                 *
 *   version 2 of the License.                                             *
 *                                                                         *
 *   As a special exception, you may use this file as part of a free       *
 *   software library without restriction.  Specifically, if other files   *
 *   instantiate templates or use macros or inline functions from this     *
 *   file, or you compile this file and link it with other files to        *
 *   produce an executable, this file does not by itself cause the         *
 *   resulting executable to be covered by the GNU General Public          *
 *   License.  This exception does not however invalidate any other        *
 *   reasons why the executable file might be covered by the GNU General   *
 *   Public License.                                                       *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef ORO_CORELIB_TYPEINFO_HPP
#define ORO_CORELIB_TYPEINFO_HPP

#include <vector>
#include <string>
#include "../base/DataSourceBase.hpp"

namespace RTT
{ namespace types {


    /**
     * A class for representing a user type, and which can build
     * instances of that type.
     */
    class RTT_API TypeInfo
    {
    protected:
        typedef std::vector<TypeBuilder*> Constructors;
        typedef std::vector<TypeTransporter*> Transporters;
        Constructors constructors;
        Transporters transporters;
    public:
        virtual ~TypeInfo();
        /**
         * Return unique the type name.
         */
        virtual const std::string& getTypeName() const = 0;

        /**
         * @name Type building/factory functions
         * Used to create objects that hold data of a certain type.
         * @{
         */
        /**
         * Build a non modifyable instance of this type.
         * @param sizehint For variable size instances, use it to hint
         * the size of the instance.
         */
        virtual base::AttributeBase* buildConstant(std::string name,base::DataSourceBase::shared_ptr, int sizehint) const;
        virtual base::AttributeBase* buildConstant(std::string name,base::DataSourceBase::shared_ptr) const = 0;
        /**
         * Build a modifyable instance of this type.
         * @param sizehint For variable size instances, use it to hint
         * the size of the instance.
         */
        virtual base::AttributeBase* buildVariable(std::string name,int sizehint) const;
        virtual base::AttributeBase* buildVariable(std::string name) const = 0;

        /**
         * Constructor syntax: construct a internal::DataSource which returns an instance of data
         * depending on the given arguments.  When \a args is empty, the default 'variable'
         * is returned.
         */
        virtual base::DataSourceBase::shared_ptr construct(const std::vector<base::DataSourceBase::shared_ptr>& args) const;

        /**
         * Automatic conversion: convert a internal::DataSource to this type.
         * For example, for converting float -> double. If no valid conversion was found,
         * returns arg again, otherwise, a new data source.
         */
        virtual base::DataSourceBase::shared_ptr convert(base::DataSourceBase::shared_ptr arg) const;

        /**
         * Add a constructor/convertor object.
         */
        virtual void addConstructor(TypeBuilder* tb);

        /**
         * build an alias with b as the value.  If b is of the wrong type,
         * 0 will be returned..
         */
        virtual base::AttributeBase* buildAlias(std::string name, base::DataSourceBase::shared_ptr b ) const = 0;

        /**
         * Build a Property of this type.
         */
        virtual base::PropertyBase* buildProperty(const std::string& name, const std::string& desc, base::DataSourceBase::shared_ptr source = 0) const = 0;

        /**
         * Build an Attribute of this type.
         */
        virtual base::AttributeBase* buildAttribute(std::string name, base::DataSourceBase::shared_ptr source = 0 ) const = 0;

        /**
         * Build a internal::ValueDataSource of this type.
         */
        virtual base::DataSourceBase::shared_ptr buildValue() const = 0;

        /** Build a internal::ReferenceDataSource of this type, pointing to the given
         * pointer
         */
        virtual base::DataSourceBase::shared_ptr buildReference(void* ptr) const = 0;

        /** @} */

        /**
         * @name Conversion to/from text
         * Used to convert data to human readable text and vice versa.
         * @{
         */
        /**
         * Output this datasource as a human readable string.
         * The default just writes the type name in parentheses to \a os.
         */
        virtual std::ostream& write(std::ostream& os, base::DataSourceBase::shared_ptr in ) const = 0;

        /**
         * Read a new value for this datasource from a human readable string.
         * The default does nothing.
         */
        virtual std::istream& read(std::istream& os, base::DataSourceBase::shared_ptr out ) const = 0;

        /**
         * Usability function which converts data to a string.
         */
        virtual std::string toString( base::DataSourceBase::shared_ptr in ) const;

        /**
         * Usability function which converts a string to data.
         */
        virtual bool fromString( const std::string& value, base::DataSourceBase::shared_ptr out ) const;
        /** @} */

        /**
         * @name Inspecting data structures.
         * Used to write a complex type to an external representation, like XML.
         * @{
         */
        /**
         * Decompose a structure as basic components into a PropertyBag.
         * @retval true decomposition resulted in new types added to targetbag.
         * @retval false nothing was added to targetbag.
         */
        virtual bool decomposeType( base::DataSourceBase::shared_ptr source, PropertyBag& targetbag ) const = 0;

        /**
         * Compose a structure from a base::DataSourceBase containing its basic components.
         * The default behavior tries to assign \a source to \a target. If this does
         * not work, because source and target have different type, this function returns false.
         */
        virtual bool composeType( base::DataSourceBase::shared_ptr source, base::DataSourceBase::shared_ptr target) const = 0;
        /**
         * @}
         */

        /**
         * @name Distribution of objects
         * Used to transport data over a network.
         * @{
         */

        /**
         * Register a protocol for data transport over a network.
         */
        bool addProtocol(int protocol_id, TypeTransporter* tt);

        TypeTransporter* getProtocol(int protocol_id) const;

        /**
         * Return a list of protocols that have transporters
         */
        std::vector<int> getTransportNames() const;

        virtual std::string getTypeIdName() const = 0;

        /**
         * Returns a new InputPort<T> object where T is the type represented by
         * this TypeInfo object.
         */
        virtual base::InputPortInterface* inputPort(std::string const& name) const = 0;

        /**
         * Returns a new OutputPort<T> object where T is the type represented by
         * this TypeInfo object.
         */
        virtual base::OutputPortInterface* outputPort(std::string const& name) const = 0;

        /**
         * Creates single data or buffered storage for this type.
         *
         * @param policy Describes the kind of storage requested by the user
         * @return a storage element.
         */
        virtual base::ChannelElementBase* buildDataStorage(ConnPolicy const& policy) const = 0;
        virtual base::ChannelElementBase* buildChannelOutput(base::InputPortInterface& port) const = 0;
        virtual base::ChannelElementBase* buildChannelInput(base::OutputPortInterface& port) const = 0;
};

}}

#endif
