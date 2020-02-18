

// class SearchableStore 
		public static void main(String[] args) {
  		Searchable searchable = new Searchable();

			searchable.search(criteria, fieldArray, data -> {
                counter.increment();
                countSinceLastSend.incrementAndGet();
                LOGGER.trace(() -> String.format("data: [%s]", Arrays.toString(data)));

								// ******* //
                // Give the data array to each of our coprocessors
                coprocessorMap.values().forEach(coprocessor ->
                        coprocessor.receive(data));
                // Send what we have every 1s or when the batch reaches a set size
                long now = System.currentTimeMillis();
                if (now >= nextProcessPayloadsTime.get() ||
                        countSinceLastSend.get() >= resultHandlerBatchSize) {

                    LOGGER.debug(LambdaLogUtil.message("{} vs {}, {} vs {}",
                            now, nextProcessPayloadsTime,
                            countSinceLastSend.get(), resultHandlerBatchSize));

                    processPayloads(resultHandler, coprocessorMap);
                    taskContext.setName(TASK_NAME);
                    taskContext.info(() -> searchKey +
                            " - running database query (" + counter.longValue() + " rows fetched)");
                    nextProcessPayloadsTime.set(Instant.now().plus(RESULT_SEND_INTERVAL).toEpochMilli());
                    countSinceLastSend.set(0);
                }
            });
		}


    public void search(final ExpressionCriteria criteria, final AbstractField[] fields, final Consumer<Val[]> consumer) {
        final List<AbstractField> fieldList = Arrays.asList(fields);
        final int nodeTermCount = ExpressionUtil.termCount(criteria.getExpression(), ProcessorTaskDataSource.NODE_NAME);
        final boolean nodeValueExists = fieldList.stream().anyMatch(Predicate.isEqual(ProcessorTaskDataSource.NODE_NAME));
        final int feedTermCount = ExpressionUtil.termCount(criteria.getExpression(), ProcessorTaskDataSource.FEED_NAME);
        final boolean feedValueExists = fieldList.stream().anyMatch(Predicate.isEqual(ProcessorTaskDataSource.FEED_NAME));
        final int pipelineTermCount = ExpressionUtil.termCount(criteria.getExpression(), ProcessorTaskDataSource.PIPELINE_UUID);
        final boolean pipelineValueExists = fieldList.stream().anyMatch(Predicate.isEqual(ProcessorTaskDataSource.PIPELINE_UUID));

        final PageRequest pageRequest = criteria.getPageRequest();
        final Condition condition = expressionMapper.apply(criteria.getExpression());
        final OrderField[] orderFields = JooqUtil.getOrderFields(FIELD_MAP, criteria);
        final List<Field<?>> dbFields = new ArrayList<>(valueMapper.getFields(fieldList));
        final Mapper[] mappers = valueMapper.getMappers(fields);

        JooqUtil.context(processorDbConnProvider, context -> {
            int offset = 0;
            int numberOfRows = 1000000;

            if (pageRequest != null) {
                offset = pageRequest.getOffset().intValue();
                numberOfRows = pageRequest.getLength();
            }

            var select = context.select(dbFields).from(PROCESSOR_TASK);
            if (nodeTermCount > 0 || nodeValueExists) {
                select = select.leftOuterJoin(PROCESSOR_NODE).on(PROCESSOR_TASK.FK_PROCESSOR_NODE_ID.eq(PROCESSOR_NODE.ID));
            }
            if (feedTermCount > 0 || feedValueExists) {
                select = select.leftOuterJoin(PROCESSOR_FEED).on(PROCESSOR_TASK.FK_PROCESSOR_FEED_ID.eq(PROCESSOR_FEED.ID));
            }
            if (pipelineTermCount > 0 || pipelineValueExists) {
                select = select.join(PROCESSOR_FILTER).on(PROCESSOR_TASK.FK_PROCESSOR_FILTER_ID.eq(PROCESSOR_FILTER.ID));
                select = select.join(PROCESSOR).on(PROCESSOR_FILTER.FK_PROCESSOR_ID.eq(PROCESSOR.ID));
            }

            try (final Cursor<?> cursor = select
                    .where(condition)
                    .orderBy(orderFields)
                    .limit(offset, numberOfRows)
                    .fetchLazy()) {

                while (cursor.hasNext()) {
                    final Result<?> result = cursor.fetchNext(1000);

                    result.forEach(r -> {
                        final Val[] arr = new Val[fields.length];
                        for (int i = 0; i < fields.length; i++) {
                            Val val = ValNull.INSTANCE;
                            final Mapper<?> mapper = mappers[i];
                            if (mapper != null) {
                                val = mapper.map(r);
                            }
                            arr[i] = val;
                        }
                        consumer.accept(arr);
                    });
                }
            }
        });
    }


    public void receive(final Val[] values) {
        final Long longStreamId = getLong(values, fieldIndexes[0]);
        final Long longEventId = getLong(values, fieldIndexes[1]);

        if (longStreamId != null && longEventId != null) {
            final EventRef ref = new EventRef(longStreamId, longEventId);

            eventRefsLock.lock();
            try {
                if (eventRefs == null) {
                    eventRefs = new EventRefs(minEvent, maxEvent, maxStreams, maxEvents, maxEventsPerStream);
                }

								// ******** //
                eventRefs.add(ref);
                this.maxEvent = eventRefs.getMaxEvent();

            } finally {
                eventRefsLock.unlock();
            }
        }
    }



    public void add(final EventRef ref) {
        if ((ref.getStreamId() > minEvent.getStreamId()
                || (ref.getStreamId() == minEvent.getStreamId() && ref.getEventId() >= minEvent.getEventId()))
                && (ref.getStreamId() < maxEvent.getStreamId() || (ref.getStreamId() == maxEvent.getStreamId()
                && ref.getEventId() <= maxEvent.getEventId()))) {
            list.add(ref);

            // Trim if the list gets bigger than double the number of events.
            if (list.size() > (maxEvents * 2)) {
                trim();
            }
        }
    }
