// (1)
    public void test() {
        doInJOOQ(sql -> {
            sql
            .deleteFrom(table("post"))
            .execute();

            assertEquals(1, sql
            .insertInto(table("post")).columns(field("id"), field("title"))
            .values(1L, "High-Performance Java Persistence")
            .execute());

            List<String> titles = sql
                .select(POST.TITLE)
                .from(POST)
                .where(POST.ID.eq(1L))
                .fetch(POST.TITLE);
            assertEquals(1, titles.size());
        });

        doInJDBC(connection -> {
            DSLContext sql = DSL.using(
                connection,
                sqlDialect(),
                new Settings().withStatementType(StatementType.STATIC_STATEMENT)
            );

            List<String> titles = sql
            .select(POST.TITLE)
            .from(POST)
            .where(POST.ID.eq(1L))
            .fetch(POST.TITLE);

////////
            assertEquals(1, titles.size());
        });

    }




// (2)
		List<Category> categoryList = repository.getCategoriesWithAgeGroup(AgeGroup._3to8);

		assertThat(categoryList) //
				.extracting(Category::getName, Category::getDescription, Category::getAgeGroup) //
				.containsExactly( //
						tuple(cars.getName(), cars.getDescription(), cars.getAgeGroup()) //
				);
