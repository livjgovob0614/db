mport java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class ArticleService {

    @Autowired
    TagService tagService;

    @Autowired
    ArticleTagService articleTagService;

    @Override
    public Article save(Article article) {
//        String [] tagIds = article.getTagIds();
        article = super.save(article);
        this.update(article);
//        String tagNames = saveArticleTags(article.getId(), tagIds);
        return article;
    }

    public Article update(Article entity) {
        String tagNames = saveArticleTags(entity.getId(), entity.getTagIds());
        entity.setTagNames(tagNames);
        entity.setUpdateTime(currentTime());
        entity.setCreateTime(null);
        if (entity.getBanner() == null) {
           entity.setBanner("");
        }
        return super.update(entity);
    }
}
