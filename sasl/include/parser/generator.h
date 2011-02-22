#ifndef SASL_PARSER_GENERATOR_H
#define SASL_PARSER_GENERATOR_H

#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

#include <sasl/include/parser/parser_forward.h>

namespace sasl{
	namespace common{
		struct token_t;
	}
}

BEGIN_NS_SASL_PARSER();

class attribute_visitor{
};

//////////////////////////////////////////////////////////////////////////
// Attributes is generated by parser. They are organized as a tree.
class attribute{
public:
	attribute();
	virtual ~attribute();

	virtual void accept( attribute_visitor& v, boost::any& ctxt ) = 0;
	virtual intptr_t rule_id() const;
	virtual void rule_id( intptr_t id );
protected:
	intptr_t rid;
};

class terminal_attribute: public attribute{
public:
	virtual void accept( attribute_visitor& v, boost::any& ctxt );
	boost::shared_ptr<sasl::common::token_t> tok;
};

class sequence_attribute: public attribute{
public:
	virtual void accept( attribute_visitor& v, boost::any& ctxt );
	std::vector< boost::shared_ptr<attribute> > attrs;
};

class selector_attribute: public attribute{
public:
	selector_attribute();
	virtual void accept( attribute_visitor& v, boost::any& ctxt );
	boost::shared_ptr<attribute> attr;
	int selected_idx;
};

class queuer_attribute: public attribute{
public:
	virtual void accept( attribute_visitor& v, boost::any& ctxt );
	std::vector< boost::shared_ptr<attribute> > attrs;
};

typedef boost::shared_ptr< sasl::common::token_t > token_ptr;
typedef std::vector< token_ptr > token_seq;
typedef token_seq::iterator token_iterator;

//////////////////////////////////////////////////////////////////////////
// Parser combinators.
class parser{
public:
	virtual bool parse( token_iterator& iter, token_iterator end, boost::shared_ptr<attribute>& attr ) const = 0;
	virtual boost::shared_ptr<parser> clone() const = 0;
	virtual ~parser(){}
};

class terminal: public parser{
public:
	terminal( size_t tok_id );
	terminal( terminal const& rhs );
	bool parse( token_iterator& iter, token_iterator end, boost::shared_ptr<attribute>& attr ) const;
	boost::shared_ptr<parser> clone() const;
private:
	terminal& operator = (terminal const &);
	size_t tok_id;
};

class repeater: public parser{

public:
	static size_t const unlimited;

	repeater( size_t lower_bound, size_t upper_bound, boost::shared_ptr<parser> expr);
	repeater( repeater const& rhs );
	bool parse( token_iterator& iter, token_iterator end, boost::shared_ptr<attribute>& attr ) const;
	boost::shared_ptr<parser> clone() const;

private:
	size_t lower_bound;
	size_t upper_bound;
	boost::shared_ptr<parser> expr;
};

class selector: public parser{

public:
	selector();
	selector( selector const& rhs );

	selector& add_branch( boost::shared_ptr<parser> p );
	std::vector< boost::shared_ptr<parser> > const& branches() const;

	bool parse( token_iterator& iter, token_iterator end, boost::shared_ptr<attribute>& attr ) const;
	boost::shared_ptr<parser> clone() const;
private:
	std::vector< boost::shared_ptr<parser> > slc_branches;
};

class queuer: public parser{
public:
	queuer();
	queuer( queuer const& rhs );

	queuer& append( boost::shared_ptr<parser> p );

	std::vector< boost::shared_ptr<parser> > const& exprs() const;

	bool parse( token_iterator& iter, token_iterator end, boost::shared_ptr<attribute>& attr ) const;
	boost::shared_ptr<parser> clone() const;
private:
	std::vector< boost::shared_ptr<parser> > exprlst;
};

class rule : public parser{
public:
	rule();
	rule( intptr_t id );
	rule( boost::shared_ptr<parser> expr, intptr_t id = -1 );
	rule( rule const& rhs );
	rule( parser const& rhs);
	rule& operator = ( parser const& rhs );
	rule& operator = ( rule const& rhs );

	intptr_t id() const;
	std::string const& name() const;
	void name( std::string const & v );

	bool parse( token_iterator& iter, token_iterator end, boost::shared_ptr<attribute>& attr ) const;
	boost::shared_ptr<parser> clone() const;
private:
	intptr_t preset_id;
	boost::shared_ptr<parser> expr;
	std::string rule_name;
};

class rule_wrapper: public parser{
public:
	rule_wrapper( rule_wrapper const& rhs );
	rule_wrapper( rule const & rhs );
	bool parse( token_iterator& iter, token_iterator end, boost::shared_ptr<attribute>& attr ) const;
	boost::shared_ptr<parser> clone() const;
private:
	rule_wrapper& operator = ( rule_wrapper const & );
	rule const& r;
};

//////////////////////////////////////////////////////////////////////////
// Operators for building parser combinator.
repeater operator * ( parser const & expr );
repeater operator - ( parser const& expr );
selector operator | ( parser const & expr0, parser const& expr1 );
selector operator | ( selector const & expr0, parser const& expr1 );
selector operator | ( selector const & expr0, selector const & expr1 );
queuer operator >> ( parser const& expr0, parser const& expr1 );
queuer operator >> ( queuer const& expr0, parser const& expr1 );
queuer operator >> ( queuer const& expr0, queuer const& expr1 );
queuer operator > ( parser const& expr0, parser const& expr1 );
queuer operator > ( queuer const& expr0, parser const& expr1 );
queuer operator > ( queuer const& expr0, queuer const& expr1 );

END_NS_SASL_PARSER();

#endif