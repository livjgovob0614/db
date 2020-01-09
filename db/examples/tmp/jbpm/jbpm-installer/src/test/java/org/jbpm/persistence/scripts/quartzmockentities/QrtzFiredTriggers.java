/*
 * Copyright 2017 Red Hat, Inc. and/or its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.jbpm.persistence.scripts.quartzmockentities;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.IdClass;

import org.hibernate.annotations.Type;

@Entity(name = "QRTZ_FIRED_TRIGGERS")
@IdClass(QrtzFiredTriggersId.class)
public class QrtzFiredTriggers {

    @Id
    @Column(name = "SCHED_NAME")
    private String schedulerName;

    @Id
    @Column(name = "ENTRY_ID")
    private String entryId;

    @Column(name = "TRIGGER_NAME")
    private String triggerName;

    @Column(name = "TRIGGER_GROUP")
    private String triggerGroup;

    @Column(name = "INSTANCE_NAME")
    private String instanceName;

    @Column(name = "FIRED_TIME")
    private Long firedTime;

    @Column(name = "SCHED_TIME")
    private Long scheduTime;

    @Column(name = "PRIORITY")
    private Integer priority;

    @Column(name = "STATE")
    private String state;

    @Column(name = "JOB_NAME")
    private String jobName;

    @Column(name = "JOB_GROUP")
    private String jobGroup;

    @Column(name = "IS_NONCONCURRENT")
    private Boolean isNonConcurrent;

    @Column(name = "REQUESTS_RECOVERY")
    private Boolean requestRecovery;
}
