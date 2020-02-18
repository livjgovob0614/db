public class Test {

    public void getDetail_1() {

        Article before = new Article();
        before.setStatus(Article.STATUS_PUBLISH);
        before.setTitle("before art1");
        before.setCreateTime(new Timestamp(System.currentTimeMillis() - 10000));

        Article now = new Article();
        now.setStatus(Article.STATUS_PUBLISH);
        now.setTitle("art1");
        now.setCreateTime(new Timestamp(System.currentTimeMillis()));

        Article next = new Article();
        next.setStatus(Article.STATUS_PUBLISH);
        next.setTitle("next art1");
        next.setCreateTime(new Timestamp(System.currentTimeMillis() + 10000));

        Article next2 = new Article();
        next2.setTitle("next 222 art1");
        next2.setStatus(Article.STATUS_PUBLISH);
        next2.setCreateTime(new Timestamp(System.currentTimeMillis() + 50000));


        articleService.save(before);
        articleService.save(now);
        articleService.save(next);
        articleService.save(next2);

        ArticleDetail detail  = articleService.getDetail_2(now.getId());
        Assert.assertTrue("not find next" + detail.getNextId(), detail.getNextId().equals(next.getId()));
        Assert.assertTrue("not find before" + detail.getNextId(), detail.getBeforeId().equals(before.getId()));
        detail = articleService.getDetail(next2.getId());
        Assert.assertNotNull("must find detail", detail.getId());
        Assert.assertEquals("not before eq", detail.getBeforeId(), next.getId());
        Assert.assertNull("next must null now" + detail.getNextId(), detail.getNextId());
    }

    public ArticleDetail getDetail_2(String id) {
        CArticle article = C_ARTICLE.as("a");
        CArticle before = C_ARTICLE.as("b");
        CArticle next = C_ARTICLE.as("n");
        CArticle inner = C_ARTICLE.as("i");
        ArticleDetail a = dao.execute(e -> {
            return e.select(Fields.all(article.fields(),
                before.ID.as("beforeId"),
                before.TITLE.as("beforeTitle"),
                next.ID.as("nextId"),
                next.TITLE.as("nextTitle")
                )).from(article)
                .leftJoin(before).on(before.ID.eq(e.select(inner.ID).from(inner).where(article.CREATE_TIME.ge(inner.CREATE_TIME)).and(inner.ID.ne(article.ID).and(inner.STATUS.eq(Article.STATUS_PUBLISH))).orderBy(inner.CREATE_TIME.desc()).limit(0, 1)))
                .leftJoin(next).on((next.ID.eq(e.select(inner.ID).from(inner).where(article.CREATE_TIME.le(inner.CREATE_TIME)).and(inner.ID.ne(article.ID).and(inner.STATUS.eq(Article.STATUS_PUBLISH))).orderBy(inner.CREATE_TIME).limit(0, 1))))
                .where(article.ID.eq(id))
                .fetchOne(r -> {
                    return dao.mapperEntityEx(r, ArticleDetail.class);
                });
        });
        List<Tag> tags = articleTagService.findTags(a.getId());
        a.setTags(tags);
        return a;
    }

    public List<Tag> findTags(String articleId) {
        return dao.execute(e -> {
            return e.select(C_TAG.fields()).from(C_TAG)
            .leftJoin(C_ARTICLE_TAG).on(C_ARTICLE_TAG.TAG_ID.eq(C_TAG.ID))
            .where(C_ARTICLE_TAG.ARTICLE_ID.eq(articleId))
            .fetchInto(Tag.class);
        });
    }

}
