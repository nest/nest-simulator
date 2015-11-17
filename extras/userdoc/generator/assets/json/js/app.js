'use strict';

/* App Module */

var nestApp = angular.module('nestApp', [
  'ngRoute',
  'nestcmdcatControllers',
  'nestcmdcatFilters',
  'nestcmdcatServices'
]);

nestApp.config(['$routeProvider',
  function($routeProvider) {
    $routeProvider.
      when('/nestcmds', {
        templateUrl: 'partials/nestcmd-list.html',
        controller: 'NestcmdListCtrl'
      }).
      when('/nestcmds/:nestcmdName', {
        templateUrl: 'partials/nestcmd-detail.html',
        controller: 'NestcmdDetailCtrl'
      }).
      otherwise({
        redirectTo: '/nestcmds'
      });    
}]);

/* Controllers */


var nestcmdcatControllers = angular.module('nestcmdcatControllers', []);

nestcmdcatControllers.controller('NestcmdListCtrl', ['$scope', 'Nestcmd',
    function($scope, Nestcmd) {
        $scope.nestcmds = Nestcmd.query();
	$scope.orderProp = 'Name';
}]);

nestcmdcatControllers.controller('NestcmdDetailCtrl', ['$scope', '$routeParams', 'Nestcmd',
    function($scope, $routeParams, Nestcmd) {
        $scope.nestcmd = Nestcmd.get({nestcmdName: $routeParams.nestcmdName});
	

  }]);


/* Services */

var nestcmdcatServices = angular.module('nestcmdcatServices', ['ngResource']);

nestcmdcatServices.factory('Nestcmd', ['$resource',
  function($resource){
    return $resource('json/:nestcmdName.json', {}, {
      query: {method:'GET', params:{nestcmdName:'0_index'}, isArray:true}
    });
}]);

 /* Filters */

angular.module('nestcmdcatFilters', []).filter('checkmark', function() {
  return function(input) {
    return input ? '\u2713' : '\u2718';
  };
});
