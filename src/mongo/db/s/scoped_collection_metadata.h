/**
 *    Copyright (C) 2018-present MongoDB, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the Server Side Public License, version 1,
 *    as published by MongoDB, Inc.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    Server Side Public License for more details.
 *
 *    You should have received a copy of the Server Side Public License
 *    along with this program. If not, see
 *    <http://www.mongodb.com/licensing/server-side-public-license>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the Server Side Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#pragma once

#include "mongo/db/s/collection_metadata.h"

namespace mongo {

/**
 * Contains the parts of the sharding state for a particular collection, which do not change due to
 * chunk move, split and merge. The implementation is allowed to be tighly coupled with the
 * CollectionShardingState from which it was derived and because of this it must not be accessed
 * outside of a collection lock.
 */
class ScopedCollectionDescription {
public:
    class Impl {
    public:
        virtual ~Impl() = default;

        virtual const CollectionMetadata& get() = 0;

    protected:
        Impl() = default;
    };

    ScopedCollectionDescription(std::shared_ptr<Impl> impl) : _impl(std::move(impl)) {}

    const auto& get() const {
        return _impl->get();
    }

    const auto* operator-> () const {
        return &get();
    }

    const auto& operator*() const {
        return get();
    }

    bool isSharded() const {
        return _impl->get().isSharded();
    }

    ChunkVersion getShardVersion() const {
        return _impl->get().getShardVersion();
    }

    ChunkVersion getCollVersion() const {
        return _impl->get().getCollVersion();
    }

    bool isValidKey(const BSONObj& key) const {
        return _impl->get().isValidKey(key);
    }

    const BSONObj& getKeyPattern() const {
        return _impl->get().getKeyPattern();
    }

    const std::vector<std::unique_ptr<FieldRef>>& getKeyPatternFields() const {
        return _impl->get().getKeyPatternFields();
    }

    BSONObj getMinKey() const {
        return _impl->get().getMinKey();
    }

    BSONObj getMaxKey() const {
        return _impl->get().getMaxKey();
    }

    BSONObj extractDocumentKey(const BSONObj& doc) const {
        return _impl->get().extractDocumentKey(doc);
    }

    bool uuidMatches(UUID uuid) const {
        return _impl->get().uuidMatches(uuid);
    }

    const BSONObj extractShardKeyFromDoc(const BSONObj& doc) const {
        return _impl->get().getChunkManager()->getShardKeyPattern().extractShardKeyFromDoc(doc);
    }

protected:
    std::shared_ptr<Impl> _impl;
};

/**
 * Contains the parts of the sharding state for a particular collection, which can change due to
 * chunk move, split and merge and represents a snapshot in time of these parts, specifically the
 * chunk ownership. The implementation is allowed to be tightly coupled with the
 * CollectionShardingState from which it was derived, but it must be allowed to be accessed outside
 * of collection lock.
 */
class ScopedCollectionFilter : public ScopedCollectionDescription {
public:
    ScopedCollectionFilter(std::shared_ptr<Impl> impl)
        : ScopedCollectionDescription(std::move(impl)) {}

    ScopedCollectionFilter(ScopedCollectionDescription&& scopedMetadata)
        : ScopedCollectionDescription(std::move(scopedMetadata)) {}

    bool keyBelongsToMe(const BSONObj& key) const {
        return _impl->get().keyBelongsToMe(key);
    }

    Chunk findIntersectingChunkWithSimpleCollation(const BSONObj& shardKey) const {
        return _impl->get().getChunkManager()->findIntersectingChunkWithSimpleCollation(shardKey);
    }
};

}  // namespace mongo
