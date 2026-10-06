<script setup lang="ts">
</script>

<template>
  <router-view v-slot="{ Component }">
    <component :is="Component" class="route-view" />
  </router-view>
</template>

<style scoped>
.route-view {
  display: flex;
  flex-direction: column;
  height: 100%;
  min-height: 0;
}
</style>
