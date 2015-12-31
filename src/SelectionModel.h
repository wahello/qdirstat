/*
 *   File name: SelectionModel.h
 *   Summary:	Handling of selected items
 *   License:	GPL V2 - See file LICENSE for details.
 *
 *   Author:	Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#ifndef SelectionModel_h
#define SelectionModel_h


#include <QItemSelectionModel>

#include "FileInfo.h"


namespace QDirStat
{
    class FileInfo;
    class DirTreeModel;

    /**
     * Selection model that can translate between QModelIndex and FileInfo
     * pointers for use with a QModelIndex based Qt item view (e.g., a
     * TreeView) and any QDirStat::DirTree based view (e.g., the
     * QDirStat::TreeMapView).
     *
     * This is only a thin wrapper around QItemSelectionModel. The
     * QItemSelectionModel base class is the master with its QModelIndex based
     * selection; this subclass fetches that QModelIndex selection and
     * translates each item into a FileInfo pointer on demand.
     **/
    class SelectionModel: public QItemSelectionModel
    {
	Q_OBJECT

    public:
	/**
	 * Create a SelectionModel that uses the DirTree in 'dirTreeModel'.
	 * This object does not take ownership of 'dirTreeModel'.
	 **/
	SelectionModel( DirTreeModel * dirTreeModel, QObject * parent = 0 );

	/**
	 * Destructor.
	 **/
	virtual ~SelectionModel();

	/**
	 * Return all currently selected items as a set.
	 **/
	FileInfoSet selectedItems();

	/**
	 * Return the current item (the one that has the keyboard focus).
	 * This might return 0 if currently no item has the keyboard focus.
	 **/
	FileInfo * currentItem() const { return _currentItem; }

	/**
	 * Return the DirTreeModel of this object.
	 **/
	DirTreeModel * dirTreeModel() const { return _dirTreeModel; }

    public slots:

	/**
	 * Replace the current selection with one item.
	 * If this item is 0, everything is deselected.
	 * This does NOT change the current item.
	 **/
	void selectItem( FileInfo * item );

	/**
	 * Extend the current selection with one item: Add this item to the set
	 * of selected items. If this item is 0, the selection remains
	 * unchanged.
	 *
	 * This does NOT change the current item.
	 *
	 * If 'clear' is 'true', this will clear the old selection first, so
	 * this has the same effect as selectItem().
	 **/
	void extendSelection( FileInfo * item, bool clear = false );

	/**
	 * Set the selected items, i.e., replace the complete selection.
	 **/
	void setSelectedItems( const FileInfoSet  & selectedItems );

	/**
	 * Make 'item' the current item. This is different from the selection:
	 * There is one current item (mostly for the keyboard focus), but there
	 * can be any number of selected items.
	 *
	 * The current item can change the selection: In the tree view in
	 * 'extended selection' mode, [Shift]+[Click] extends the range of
	 * selected items (and makes the clicked item the current item),
	 * [Ctrl]+[Click] toggles the selected state of an item (and makes it
	 * the current item).
	 *
	 * 'item' may be 0. In that case, there is no current item.
	 *
	 * If 'select' is 'true', this also implicitly replaces the selection
	 * with this item, i.e. only this item is selected afterwards. If
	 * 'select' is 'false', the selection remains unchanged.
	 **/
	void setCurrentItem( FileInfo * item, bool select = false );

	/**
	 * For debugging: Dump the currently selected items and the current
	 * item to the log.
	 **/
	void dumpSelectedItems();

    signals:

	/**
	 * Emitted when the current item changes. 'newCurrent' is the new
	 * current item, 'oldCurrent' the previous one. Any of them might be 0.
	 **/
	void currentItemChanged( FileInfo * newCurrent, FileInfo * oldCurrent );

	/**
	 * Emitted when the selection changes.
	 **/
	void selectionChanged();
	void selectionChanged( const FileInfoSet & selectedItems );

    protected slots:

	/**
	 * Propagate the QModelIndex based currentChanged() signal to
	 * the FileInfo * based one
	 **/
	void propagateCurrentChanged( const QModelIndex & newCurrent,
				      const QModelIndex & oldCurrent );

	/**
	 * Propagate the QModelIndex based selectionChanged() signal to
	 * the FileInfo * based one
	 **/
	void propagateSelectionChanged( const QItemSelection & selected,
					const QItemSelection & deselected );

    protected:


	// Data members

	DirTreeModel	* _dirTreeModel;
	FileInfo	* _currentItem;
	FileInfoSet	  _selectedItems;
	bool		  _selectedItemsDirty;

    };	// class SelectionModel



    /**
     * Proxy class for SelectionModel: Forward the relevant selection signals
     * to a receiver.
     *
     * The basic idea behind this is to avoid signal ping-pong between the
     * SelectionModel and any number of conncected view widgets:
     *
     * View A sends a "selectionChanged()" signal to the SelectionModel, the
     * SelectionModel sends that signal to all connected widgets - including
     * back to view A which initiated it, which then sends the signal again to
     * the model etc. etc.
     *
     * With this proxy class, the view connects the "changed" signals not from
     * the SelectionModel to itself, but from the SelectionModelProxy (which in
     * turn connects the signals transparently from the master SelectionModel).
     *
     * Now if view A sends the signal, it first blocks signals from its
     * SelectionModelProxy (preferably using a SignalBlocker), sends the signal
     * and unblocks signals again from the proxy. This means that view A does
     * not receive its own signals, but all other connected widgets do.
     *
     * If we'd just block all signals from the SelectionModel, the other
     * widgets would not get notified at all. With this approach, only the
     * connections from one widget are disabled temporarily.
     *
     * Of course, each view has to create and set up its own proxy. They cannot
     * be shared among views.
     **/
    class SelectionModelProxy: public QObject
    {
	Q_OBJECT

    public:
	/**
	 * Creates a SelectionModelProxy. This automatically connects the
	 * master SelectionModel's signals to the matching signals of this
	 * object.
	 *
	 * 'parent' is the QObject tree parent for automatic deletion
	 * of this object when the parent is deleted.
	 **/
	SelectionModelProxy( SelectionModel * master, QObject * parent = 0 );

    signals:

	// From QItemSelectionModel

	void selectionChanged( const QItemSelection & selected, const QItemSelection & deselected );
	void currentChanged	 ( const QModelIndex & current, const QModelIndex & previous );
	void currentColumnChanged( const QModelIndex & current, const QModelIndex & previous );
	void currentRowChanged	 ( const QModelIndex & current, const QModelIndex & previous );

	// from SelectionModel

	void selectionChanged();
	void selectionChanged( const FileInfoSet & selectedItems );
	void currentItemChanged( FileInfo * newCurrent, FileInfo * oldCurrent );

    };	// class SelectionModelProxy


}	// namespace QDirStat

#endif	// SelectionModel_h