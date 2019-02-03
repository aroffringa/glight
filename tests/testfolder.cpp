#include "../libtheatre/folder.h"

#include <boost/test/unit_test.hpp>

#include <memory>

BOOST_AUTO_TEST_SUITE(folder)

BOOST_AUTO_TEST_CASE( ParentPath )
{
	BOOST_CHECK_EQUAL( "" , Folder::ParentPath("") );
	BOOST_CHECK_EQUAL( "" , Folder::ParentPath("root") );
	BOOST_CHECK_EQUAL( "root" , Folder::ParentPath("root/file") );
	BOOST_CHECK_EQUAL( "a/b" , Folder::ParentPath("a/b/c") );
	BOOST_CHECK_EQUAL( "a/a and b" , Folder::ParentPath("a/a and b/c") );
	BOOST_CHECK_EQUAL( "1/2/3" , Folder::ParentPath("1/2/3/4") );
	BOOST_CHECK_EQUAL( "a" , Folder::ParentPath(Folder::ParentPath("a/b/a")) );
}

BOOST_AUTO_TEST_CASE( LastName )
{
	BOOST_CHECK_EQUAL( "" , Folder::LastName("") );
	BOOST_CHECK_EQUAL( "root" , Folder::LastName("root") );
	BOOST_CHECK_EQUAL( "file" , Folder::LastName("root/file") );
	BOOST_CHECK_EQUAL( "c" , Folder::LastName("a/b/c") );
	BOOST_CHECK_EQUAL( "c" , Folder::LastName("a/a and b/c") );
	BOOST_CHECK_EQUAL( "4" , Folder::LastName("1/2/3/4") );
	BOOST_CHECK_EQUAL( "a" , Folder::LastName(Folder::LastName("a/b/a")) );
}

BOOST_AUTO_TEST_CASE( FollowDown )
{
	std::unique_ptr<Folder> a(new Folder("a"));
	std::unique_ptr<Folder> b(new Folder("b"));
	std::unique_ptr<Folder> c(new Folder("c"));
	c->SetParent(*b);
	b->Add(*c);
	
	b->SetParent(*a);
	a->Add(*b);
	
	BOOST_CHECK_EQUAL( &a->FollowDown("b") , b.get() );
	
	BOOST_CHECK_EQUAL( &a->FollowDown("b/c") , c.get() );
	
	BOOST_CHECK_EQUAL( &b->FollowDown("c") , c.get() );
}


BOOST_AUTO_TEST_CASE( FollowRelPath )
{
	std::unique_ptr<Folder> a(new Folder("a"));
	std::unique_ptr<Folder> b(new Folder("b"));
	std::unique_ptr<NamedObject> c(new NamedObject("c"));
	
	c->SetParent(*b);
	b->Add(*c);
	
	b->SetParent(*a);
	a->Add(*b);
	
	BOOST_CHECK_EQUAL( &a->FollowRelPath("b") , b.get() );
	
	BOOST_CHECK_EQUAL( &a->FollowRelPath("b/c") , c.get() );
	
	BOOST_CHECK_EQUAL( &b->FollowRelPath("c") , c.get() );
}

BOOST_AUTO_TEST_SUITE_END()
