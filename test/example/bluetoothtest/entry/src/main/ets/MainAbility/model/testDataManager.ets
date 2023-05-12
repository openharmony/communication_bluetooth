/**
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
let NextId = 0;

export enum CategoryManager {
  All ,
  APIManager ,
  GattManager ,
  SPPManager ,
  ScenarioManager ,
  AutoTestManager ,
  GattAutoTestManager ,
  SppAutoTestManager ,
  ProfileAutoTestManager ,
  ProfileManager ,
  BenchMarkManager
}

export class TestDataManager {
  id: string;
  name: string;
  detail: string;
  url: string;
  image: Resource
  category: CategoryManager;

  constructor( name: string , detail: string , url: string , image: Resource , category: CategoryManager ) {
    this.id = `${ NextId ++ }`;
    this.name = name;
    this.url = url;
    this.detail = detail;
    this.image = image;
    this.category = category;
  }
}

export enum CaseCategoryManager {
  BRManager ,
  BLEManager ,
  GattManager ,
  SPPManager ,
  ProfileManager ,
  GattClientManager ,
  GattServerManager ,
  SPPClientManager ,
  SPPServerManager ,
  ProfileA2dpSourceManager ,
  ProfileHandsFreeAudioGatewayManager ,
  ProfileHitHostManager ,
  ProfilePanNetworkManager ,
}

export class TestApiManager {
  id: string;
  name: string;
  detail: string;
  method: string;
  api: any;
  result: string;
  category: CaseCategoryManager;

  constructor( name: string , detail: string , method: string , api: any , result: string , category: CaseCategoryManager ) {
    this.id = `${ NextId ++ }`;
    this.name = name;
    this.detail = detail;
    this.method = method;
    this.api = api;
    this.result = result;
    this.category = category;
  }
}

export enum AutoCategoryManager {
  BrAutoManager ,
  BleAutoManager ,
  GattClientAutoManager ,
  GattServerAutoManager ,
  SppClientAutoManager ,
  SppServerAutoManager ,
  A2dpSourceAutoManager ,
  HandsFreeAudioGatewayAutoManager ,
  HitHostAutoManager ,
  PanNetworkAutoManager
}

export class TestAutoManager {
  id: string;
  name: string;
  detail: string;
  method: string;
  api: any;
  image: Resource;
  result: string;
  category: AutoCategoryManager;

  constructor( name: string , detail: string , method: string , api: any , result: string , category: AutoCategoryManager ) {
    this.id = `${ NextId ++ }`;
    this.name = name;
    this.detail = detail;
    this.method = method;
    this.api = api;
    this.result = result;
    this.category = category;
  }
}


export enum ScenarioCategoryManager {
  BrFoundScenarioManager ,
  BleFoundScenarioManager ,
  GattClientScenarioManager ,
  GattServerScenarioManager ,
  SppClientScenarioManager ,
  SppServerScenarioManager ,
}

export class TestScenarioManager {
  id: string;
  name: string;
  detail: string;
  method: string;
  api: any;
  image: Resource;
  result: string;
  category: ScenarioCategoryManager;

  constructor( name: string , detail: string , method: string , api: any , result: string , category: ScenarioCategoryManager ) {
    this.id = `${ NextId ++ }`;
    this.name = name;
    this.detail = detail;
    this.method = method;
    this.api = api;
    this.result = result;
    this.category = category;
  }
}

export enum BenchmarkCategoryManager {
  BrBenchmarkManager ,
  BleBenchmarkManager ,
  GattClientBenchmarkManager ,
  GattServerBenchmarkManager ,
  SppClientBenchmarkManager ,
  SppServerBenchmarkManager ,
}

export class TestBenchmarkManager {
  id: string;
  name: string;
  detail: string;
  method: string;
  api: any;
  image: Resource;
  result: string;
  category: BenchmarkCategoryManager;

  constructor( name: string , detail: string , method: string , api: any , result: string , category: BenchmarkCategoryManager ) {
    this.id = `${ NextId ++ }`;
    this.name = name;
    this.detail = detail;
    this.method = method;
    this.api = api;
    this.result = result;
    this.category = category;
  }
}