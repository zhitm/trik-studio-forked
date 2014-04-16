#pragma once

#include <QtCore/QHash>
#include <QtCore/QScopedPointer>

#include <qrkernel/ids.h>

#include <interpreterBase/blocksBase/blocksTableInterface.h>
#include <interpreterBase/blocksBase/blockInterface.h>

#include "src/managers/blocksFactoryManagerInterface.h"

namespace interpreterCore {
namespace interpreter {
namespace details {

/// Implementation of blocks table functionality required by InterpreterBase.
/// Provides mapping from block ids to objects that implement logic of block, also creates blocks when needed, so
/// clients can simply request a block by given id, and a block table will do the rest.
/// Also supports operations that shall be performed on all blocks in a system, such as setting failure or idle flags.
/// and objects implementing logic of that blocks.
class BlocksTable : public interpreterBase::blocksBase::BlocksTableInterface
{
public:
	/// Constructor.
	/// @param blocksFactoryManager - a factory manager that is used to create new blocks when needed.
	BlocksTable(BlocksFactoryManagerInterface &blocksFactoryManager);

	~BlocksTable() override;

	interpreterBase::blocksBase::BlockInterface *block(qReal::Id const &element) override;

	/// Clears blocks table.
	void clear();

	/// Sets "failure" state for all blocks, which allows to abort program execution: if block is failed, it will not
	/// invoke next block.
	void setFailure();

	/// Sets "idle" state for all blocks, thus preparing them for another execution.
	/// @todo possibly unneeded. It will not clean existing connections and pending signals, so it is safer to delete
	/// and re-create all blocks before another run of a program.
	void setIdleForBlocks();

private:
	QHash<qReal::Id, interpreterBase::blocksBase::BlockInterface *> mBlocks;  // Has ownership
	BlocksFactoryManagerInterface &mBlocksFactoryManager;
};

}
}
}
