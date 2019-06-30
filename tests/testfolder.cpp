#include "../theatre/folder.h"
#include "../theatre/management.h"

#include <boost/test/unit_test.hpp>

#include <memory>

BOOST_AUTO_TEST_SUITE(folder)

BOOST_AUTO_TEST_CASE( AddItem )
{
	std::unique_ptr<Folder> a(new Folder("a"));
	BOOST_CHECK_EQUAL( a->Children().size() , 0 );
	std::unique_ptr<Folder> b(new Folder("b"));
	BOOST_CHECK_EQUAL( b->Children().size() , 0 );
	a->Add(*b);
	BOOST_CHECK_EQUAL( a->Children().size() , 1 );
	BOOST_CHECK_EQUAL( b->Children().size() , 0 );
	BOOST_CHECK_EQUAL( b->Parent().Name() , a->Name() );

	// Check if duplicate names in one folder generate an exception
	std::unique_ptr<Folder> b2(new Folder("b"));
	BOOST_CHECK_THROW( a->Add(*b2), std::exception );
}

BOOST_AUTO_TEST_CASE( Move )
{
	std::unique_ptr<Folder> a(new Folder("a"));
	std::unique_ptr<Folder> b(new Folder("b"));
	std::unique_ptr<Folder> c(new Folder("c"));
	a->Add(*b);
	Folder::Move(*b, *c);
	BOOST_CHECK_EQUAL( a->Children().size(), 0 );
	BOOST_CHECK_EQUAL( b->Children().size(), 0 );
	BOOST_CHECK_EQUAL( c->Children().size(), 1 );
	BOOST_CHECK_EQUAL( c->GetChild("b").Name(), "b" );
	Folder::Move(*b, *a);
	BOOST_CHECK_EQUAL( a->Children().size(), 1 );
	BOOST_CHECK_EQUAL( a->GetChild("b").Name(), "b" );
	Folder::Move(*b, *a);
}

BOOST_AUTO_TEST_CASE( FullPath )
{
	std::unique_ptr<Folder> a(new Folder("a"));
	BOOST_CHECK_EQUAL( a->FullPath(), "a");
	std::unique_ptr<Folder> b(new Folder("b"));
	BOOST_CHECK_EQUAL( b->FullPath(), "b");
	a->Add(*b);
	BOOST_CHECK_EQUAL( b->FullPath(), "a/b");
	std::unique_ptr<Folder> c(new Folder("c"));
	b->Add(*c);
	BOOST_CHECK_EQUAL( a->FullPath(), "a");
	BOOST_CHECK_EQUAL( b->FullPath(), "a/b");
	BOOST_CHECK_EQUAL( c->FullPath(), "a/b/c");
}

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

BOOST_AUTO_TEST_CASE( RemoveRoot_move )
{
	BOOST_CHECK_EQUAL( "" , Folder::RemoveRoot("") );
	BOOST_CHECK_EQUAL( "" , Folder::RemoveRoot("root") );
	BOOST_CHECK_EQUAL( "file" , Folder::RemoveRoot("root/file") );
	BOOST_CHECK_EQUAL( "b/c" , Folder::RemoveRoot("a/b/c") );
	BOOST_CHECK_EQUAL( "a and b/c" , Folder::RemoveRoot("a/a and b/c") );
	BOOST_CHECK_EQUAL( "2/3/4" , Folder::RemoveRoot("1/2/3/4") );
	BOOST_CHECK_EQUAL( "b/a" , Folder::RemoveRoot("a/b/a") );
}

BOOST_AUTO_TEST_CASE( RemoveRoot_ref )
{
	std::string path = "";
	BOOST_CHECK_EQUAL( "" , Folder::RemoveRoot(path) );
	path = "root";
	BOOST_CHECK_EQUAL( "" , Folder::RemoveRoot(path) );
	path = "root/file";
	BOOST_CHECK_EQUAL( "file" , Folder::RemoveRoot(path) );
	path = "a/b/c";
	BOOST_CHECK_EQUAL( "b/c" , Folder::RemoveRoot(path) );
	path = "a/a and b/c";
	BOOST_CHECK_EQUAL( "a and b/c" , Folder::RemoveRoot(path) );
	path = "1/2/3/4";
	BOOST_CHECK_EQUAL( "2/3/4" , Folder::RemoveRoot(path) );
	path = "a/b/a";
	BOOST_CHECK_EQUAL( "b/a" , Folder::RemoveRoot(path) );
}

BOOST_AUTO_TEST_CASE( FollowDown )
{
	std::unique_ptr<Folder> a(new Folder("a"));
	std::unique_ptr<Folder> b(new Folder("b"));
	std::unique_ptr<Folder> c(new Folder("c"));
	b->Add(*c);
	a->Add(*b);
	
	BOOST_CHECK_EQUAL( a->FollowDown("b") , b.get() );
	
	BOOST_CHECK_EQUAL( a->FollowDown("b/c") , c.get() );
	
	BOOST_CHECK_EQUAL( b->FollowDown("c") , c.get() );
}


BOOST_AUTO_TEST_CASE( FollowRelPath )
{
	std::unique_ptr<Folder> a(new Folder("a"));
	std::unique_ptr<Folder> b(new Folder("b"));
	std::unique_ptr<FolderObject> c(new FolderObject("c"));
	
	b->Add(*c);
	a->Add(*b);
	
	BOOST_CHECK_EQUAL( a->FollowRelPath("b") , b.get() );
	
	BOOST_CHECK_EQUAL( a->FollowRelPath("b/c") , c.get() );
	
	BOOST_CHECK_EQUAL( b->FollowRelPath("c") , c.get() );
}

BOOST_AUTO_TEST_CASE( FolderManagement )
{
	Management management;
	Folder& root = management.RootFolder();
	BOOST_CHECK(root.Children().empty());
	root.SetName("a");
	BOOST_CHECK_EQUAL(&management.GetObjectFromPath("a"), &root);
	Folder& folderB = management.AddFolder(root);
	folderB.SetName("b");
	BOOST_CHECK_EQUAL(&management.GetObjectFromPath("a/b"), &folderB);
}

BOOST_AUTO_TEST_SUITE_END()
