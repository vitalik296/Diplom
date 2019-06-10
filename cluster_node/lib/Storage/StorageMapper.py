from utils import BasicMapper


class StorageMapper(BasicMapper):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._queries.update({"get_package": """
                                        SELECT
                                            block_offset, inblock_offset, real_size
                                        FROM
                                            packages
                                        WHERE
                                            package_id=?;
                                             """,
                              "update_package": """
                                        UPDATE
                                            packages
                                        SET
                                            package_id=?,
                                            inblock_offset=?,
                                            real_size=?
                                        WHERE
                                            id=?;
                                                """,
                              "initial_insert": """
                                        INSERT INTO
                                            packages(block_offset)
                                        VALUES
                                            (?);
                                                """,
                              "get_free_package": """
                                        SELECT
                                            id, block_offset, inblock_offset
                                        FROM
                                            packages
                                        WHERE
                                            real_size=0
                                        LIMIT
                                            1;
                                                  """,
                              "select_filled_package": """
                                        SELECT
                                            COUNT(*)
                                        FROM
                                            packages
                                        WHERE
                                            real_size>0;
                                                       """
                              })
